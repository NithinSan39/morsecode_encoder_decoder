/* USER CODE BEGIN Header */
/**
  * @brief : ITU Morse Decoder (Blocking/Synchronized Mode)
  * @note  : This version forces the CPU to wait for your hand movements.
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdio.h>

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// --- SETTINGS ---
#define UNIT_TIME       200      // Speed Base
#define DOT_MAX         400      // Below 400ms = DOT
#define LETTER_TIMEOUT  2000     // Wait 2 SECONDS to finish a letter (S vs E)
#define WORD_TIMEOUT    4000     // Wait 4 SECONDS to add a Space

// --- HARDWARE ---
#define INPUT_PORT      GPIOA
#define INPUT_PIN       GPIO_PIN_0
#define LED_PORT        GPIOA
#define LED_PIN         GPIO_PIN_3
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t last_release_time = 0;
char current_sequence[10];
int seq_index = 0;
uint8_t letter_finished = 1; // Flag to stop repeating letters
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN 0 */
void UART_Print(char* str) {
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}

// --- DECODER ---
const char* MORSE_MAP[] = {
    ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", "..",   ".---",
    "-.-",  ".-..", "--",   "-.",   "---",  ".--.", "--.-", ".-.",  "...",  "-",
    "..-",  "...-", ".--",  "-..-", "-.--", "--.."
};
const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void Decode_And_Print() {
    // If buffer is empty, do nothing
    if (seq_index == 0) return;

    current_sequence[seq_index] = '\0'; // Seal string

    char detected_char = '?';
    for (int i = 0; i < 26; i++) {
        if (strcmp(current_sequence, MORSE_MAP[i]) == 0) {
            detected_char = ALPHABET[i];
            break;
        }
    }

    // Print the Letter
    char buff[2] = {detected_char, '\0'};
    UART_Print(buff);

    // Reset Buffer
    seq_index = 0;
    memset(current_sequence, 0, 10);
    letter_finished = 1; // Mark as done
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  UART_Print("\r\n=== SYNCHRONIZED DECODER ===\r\n");
  UART_Print("1. Tap the wire.\r\n");
  UART_Print("2. Wait 2 seconds to see the letter.\r\n");
  UART_Print("OUTPUT: ");

  last_release_time = HAL_GetTick();
  memset(current_sequence, 0, 10);
  /* USER CODE END 2 */

  while (1)
  {
      uint32_t now = HAL_GetTick();

      // --- 1. CHECK FOR PRESS (Active Low) ---
      if (HAL_GPIO_ReadPin(INPUT_PORT, INPUT_PIN) == GPIO_PIN_RESET) {

          // A. DEBOUNCE (Wait 50ms to confirm it's real)
          HAL_Delay(50);

          // B. IF STILL PRESSED, START MEASURING
          if (HAL_GPIO_ReadPin(INPUT_PORT, INPUT_PIN) == GPIO_PIN_RESET) {

              HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); // LED ON
              uint32_t press_start = HAL_GetTick();

              // *** THE BLOCKING MAGIC ***
              // The code effectively "pauses" here as long as you hold the wire.
              // It cannot do anything else (like accidentally decode) until you let go.
              while (HAL_GPIO_ReadPin(INPUT_PORT, INPUT_PIN) == GPIO_PIN_RESET) {
                  // Just wait...
              }

              // C. BUTTON RELEASED
              uint32_t duration = HAL_GetTick() - press_start;
              HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); // LED OFF

              // D. STORE DOT OR DASH
              if (duration < DOT_MAX) {
                   if(seq_index < 9) current_sequence[seq_index++] = '.';
                   UART_Print("."); // Echo Dot
              } else {
                   if(seq_index < 9) current_sequence[seq_index++] = '-';
                   UART_Print("-"); // Echo Dash
              }

              // Reset Timers
              last_release_time = HAL_GetTick();
              letter_finished = 0; // We are building a new letter!
          }
      }

      // --- 2. CHECK FOR TIMEOUTS (Only if we aren't done yet) ---
      else {
          if (letter_finished == 0) {
              // If you have waited long enough (2 Seconds)...
              if ((now - last_release_time) > LETTER_TIMEOUT) {
                  Decode_And_Print();
                  // Note: letter_finished is set to 1 inside the function
              }
          }

          // Check for Space (4 Seconds)
          // We check 'seq_index == 0' to make sure we don't print space mid-letter
          if (seq_index == 0 && (now - last_release_time) > WORD_TIMEOUT) {
              if (letter_finished == 1) { // Only print space once
                  UART_Print(" ");
                  letter_finished = 2; // Set to 2 to prevent infinite spaces
              }
          }
      }
  }
}

// --- STANDARD INIT ---
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

  GPIO_InitStruct.Pin = INPUT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(INPUT_PORT, &GPIO_InitStruct);
}

void Error_Handler(void) { __disable_irq(); while (1) {} }
