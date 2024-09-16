#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>
#include <Ultrasonic.h>
#include <queue.h>
#include <semphr.h>
// #include <IRremote.h>

// Constants
#define QUEUE_LENGTH_DISTANCE    10
#define QUEUE_LENGTH_COMMAND     10
#define QUEUE_LENGTH_MESSAGE     1
#define STACK_SIZE               128

#define TASK_PRIORITY_READ       3
#define TASK_PRIORITY_PROCESS    2
#define TASK_PRIORITY_LED        1
#define TASK_PRIORITY_LCD        1

#define LED_PIN                  9
#define LCD_COLUMNS              16
#define LCD_ROWS                 2
#define DISTANCE_THRESHOLD       12.0f  // inches
#define IR_RECEIVE_PIN           10

// Function prototypes
void vReadDistanceTask(void *pvParameters);
void vProcessTask(void *pvParameters);
void vLEDControlTask(void *pvParameters);
void vLCDControlTask(void *pvParameters);

// Global variables
SemaphoreHandle_t xLCDSemaphore;
QueueHandle_t xQueueDistance;
QueueHandle_t xQueueCommand;
QueueHandle_t xQueueMessage;

// Initialize LCD and Ultrasonic sensor
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
Ultrasonic ultrasonic(12, 11); // (Trigger Pin, Echo Pin)

// Initialize IR receiver
// IRrecv irrecv(IR_RECEIVE_PIN);
// decode_results results;

void setup() {
    // Initialize hardware
    lcd.begin(LCD_COLUMNS, LCD_ROWS);
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(9600);
    Serial.print("Hello");

    // irrecv.enableIRIn();

    // Create queues
    xQueueDistance = xQueueCreate(QUEUE_LENGTH_DISTANCE, sizeof(float));
    xQueueCommand = xQueueCreate(QUEUE_LENGTH_COMMAND, sizeof(bool));
    xQueueMessage = xQueueCreate(QUEUE_LENGTH_MESSAGE, sizeof(float));

    // Check if queues were created successfully
    if (xQueueDistance == NULL || xQueueCommand == NULL || xQueueMessage == NULL) {
        lcd.print("Queue failed!");
        while (1); // Halt program
    }

    // Create semaphore for LCD access
    xLCDSemaphore = xSemaphoreCreateMutex();

    // Check if semaphore was created successfully
    if (xLCDSemaphore == NULL) {
        lcd.print("Semaphore failed!");
        while (1); // Halt program
    }

    // Create tasks
    xTaskCreate(vReadDistanceTask, "ReadDistance", STACK_SIZE, NULL, TASK_PRIORITY_READ, NULL);
    xTaskCreate(vProcessTask, "ProcessTask", STACK_SIZE, NULL, TASK_PRIORITY_PROCESS, NULL);
    xTaskCreate(vLEDControlTask, "LEDControl", STACK_SIZE, NULL, TASK_PRIORITY_LED, NULL);
    xTaskCreate(vLCDControlTask, "LCDControl", STACK_SIZE, NULL, TASK_PRIORITY_LCD, NULL);

    // Start the scheduler
    vTaskStartScheduler();
}

void loop() {
    // Empty. The FreeRTOS scheduler is running the tasks.
}

// Task to read distance from the ultrasonic sensor
void vReadDistanceTask(void *pvParameters) {
    (void) pvParameters;
    float distanceInches;

    for (;;) {
        // Read distance in centimeters and convert to inches
        int distanceCm = ultrasonic.distanceRead();
        distanceInches = distanceCm / 2.54f;

        // Send distance to the LCD message queue (overwrite if full)
        xQueueOverwrite(xQueueMessage, &distanceInches);

        // Send distance to the processing task
        if (xQueueSend(xQueueDistance, &distanceInches, portMAX_DELAY) != pdPASS) {
            // Handle error (e.g., log or increment error counter)
        }

        // Delay to control the sampling rate
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task to process the distance and decide whether to turn the LED on or off
void vProcessTask(void *pvParameters) {
    (void) pvParameters;
    float retrievedDistance;
    bool ledCommand;

    for (;;) {
        // Receive distance from the queue
        if (xQueueReceive(xQueueDistance, &retrievedDistance, portMAX_DELAY) == pdPASS) {
            // Determine LED command based on distance
            ledCommand = (retrievedDistance <= DISTANCE_THRESHOLD);

            // Send command to the LED control task
            if (xQueueSend(xQueueCommand, &ledCommand, portMAX_DELAY) != pdPASS) {
                // Handle error
            }
        }
    }
}

// Task to control the LED based on the command received
void vLEDControlTask(void *pvParameters) {
    (void) pvParameters;
    bool ledState;

    for (;;) {
        // Receive LED command from the queue
        if (xQueueReceive(xQueueCommand, &ledState, portMAX_DELAY) == pdPASS) {
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        }

        // Small delay to yield control
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task to update the LCD with the latest distance reading
void vLCDControlTask(void *pvParameters) {
    (void) pvParameters;
    float distance;
    char buffer[16];
    char floatString[10];

    for (;;) {
        // Receive distance from the message queue
        if (xQueueReceive(xQueueMessage, &distance, portMAX_DELAY) == pdPASS) {
            // Convert the float to a string with 2 decimal places
            dtostrf(distance, 6, 2, floatString); // width=6, precision=2

            // Format the final string
            snprintf(buffer, sizeof(buffer), "Distance: %s", floatString);

            // Protect LCD access with semaphore
            if (xSemaphoreTake(xLCDSemaphore, pdMS_TO_TICKS(5)) == pdTRUE) {
                // Update LCD without clearing (to prevent flicker)
                lcd.setCursor(0, 0);
                lcd.print(buffer);
                xSemaphoreGive(xLCDSemaphore);
            }

            // Delay outside the critical section
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}