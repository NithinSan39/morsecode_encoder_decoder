/* USER CODE BEGIN Header */
/**
  * @brief : Morse Code ENCODER (Continuous Loop)
  * @note  : ensure NVIC 'USART1 global interrupt' is ENABLED.
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdio.h>

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// --- TIMING CONFIGURATION ---
#define UNIT_MS         200
#define DOT_TIME        (1 * UNIT_MS)
#define DASH_TIME       (3 * UNIT_MS)
#define SYMBOL_GAP      (1 * UNIT_MS)
#define LETTER_GAP      (3 * UNIT_MS)
#define WORD_GAP        (7 * UNIT_MS)

// --- HARDWARE ---
#define LED_PORT        GPIOA
#define LED_PIN         GPIO_PIN_3
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t rx_data[2];
uint8_t rx_buffer[100];
uint8_t rx_index = 0;
volatile uint8_t input_ready = 0;

const char* MORSE_MAP[] = {
    ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", "..",   ".---",
    "-.-",  ".-..", "--",   "-.",   "---",  ".--.", "--.-", ".-.",  "...",  "-",
    "..-",  "...-", ".--",  "-..-", "-.--", "--.."
};
const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* NUM_MAP[] = {
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};
const char NUMBERS[] = "0123456789";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN 0 */
void UART_Print(char* str) {
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}

// --- BLINK FUNCTION ---
void Play_Morse_String(char* morse_seq) {
    int len = strlen(morse_seq);
    for (int i = 0; i < len; i++) {
        char c = morse_seq[i];

        if (c == '.') {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);   // ON
            HAL_Delay(DOT_TIME);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); // OFF
            HAL_Delay(SYMBOL_GAP);
        }
        else if (c == '-') {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);   // ON
            HAL_Delay(DASH_TIME);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); // OFF
            HAL_Delay(SYMBOL_GAP);
        }
    }
}

// --- ENCODE LOGIC ---
void Encode_And_Play(char* text) {
    UART_Print("\r\nEncoding: ");

    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') c -= 32; // To Uppercase

        const char* code_ptr = NULL;

        // Check Letters
        for(int k=0; k<26; k++) {
            if(ALPHABET[k] == c) {
                code_ptr = MORSE_MAP[k];
                break;
            }
        }
        // Check Numbers
        if (code_ptr == NULL) {
             for(int k=0; k<10; k++) {
                if(NUMBERS[k] == c) {
                    code_ptr = NUM_MAP[k];
                    break;
                }
            }
        }

        // Process Result
        if (code_ptr != NULL) {
            UART_Print((char*)code_ptr);
            UART_Print(" ");
            Play_Morse_String((char*)code_ptr);
            HAL_Delay(LETTER_GAP);
        }
        else if (c == ' ') {
            UART_Print("[SPACE] ");
            HAL_Delay(WORD_GAP);
        }
    }
    UART_Print("\r\nDone.\r\n");
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  UART_Print("\r\n=== MORSE ENCODER READY ===\r\n");
  UART_Print("Type a word (e.g., SOS) and press Enter.\r\n");

  // Enable Interrupt
  HAL_UART_Receive_IT(&huart1, rx_data, 1);
  /* USER CODE END 2 */

  while (1)
  {
      /* USER CODE BEGIN 3 */
      // Check if data is ready
      if (input_ready == 1) {

          UART_Print("\r\nReceived: ");
          UART_Print((char*)rx_buffer);

          // DO THE WORK
          Encode_And_Play((char*)rx_buffer);

          // RESET
          memset(rx_buffer, 0, 100);
          rx_index = 0;
          input_ready = 0;

          // *** ASK AGAIN ***
          UART_Print("\r\nReady for next word: ");
      }
      /* USER CODE END 3 */
  }
}

// --- STANDARD CONFIG ---
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if (rx_data[0] == '\r' || rx_data[0] == '\n') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0';
                input_ready = 1;
            }
        }
        else {
            if (rx_index < 99) {
                rx_buffer[rx_index++] = rx_data[0];
            }
        }
        HAL_UART_Receive_IT(&huart1, rx_data, 1);
    }
}
void Error_Handler(void) { __disable_irq(); while (1) {} }
/* USER CODE END 4 */
