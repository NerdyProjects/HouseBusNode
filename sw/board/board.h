/*
 * board.h
 *
 *  Created on: 13.01.2018
 *      Author: matthias
 */

#ifndef BOARD_BOARD_H_
#define BOARD_BOARD_H_

#define BOARD_NAME                  "Hausspielerei Trimatik MC base board"

#define STM32_LSECLK                0U
#define STM32_LSEDRV                (3U << 3U)
#define STM32_HSECLK                0U


/*
 * MCU type as defined in the ST header.
 */
#define STM32F072xB

#define GPIOA_PU_OFF                0U
#define GPIOA_ATS                   1U
#define GPIOA_UART2_RTS             1U
#define GPIOA_UART2_DE              1U
#define GPIOA_KTS                   2U
#define GPIOA_UART2_TX              2U
#define GPIOA_VTS                   3U
#define GPIOA_UART2_RX              3U
#define GPIOA_STS                   4U
#define GPIOA_FB1                   5U
#define GPIOA_DIN5                  5U
#define GPIOA_FB2                   6U
#define GPIOA_LED_A                 7U
#define GPIOA_LED_RED               7U
#define GPIOA_ODSW3                 8U
#define GPIOA_SW_B1_135             8U
#define GPIOA_UART1_TX              9U
#define GPIOA_K4_T2                 9U
#define GPIOA_UART1_RX              10U
#define GPIOA_SW_B2_135             10U
#define GPIOA_USB_DM                11U
#define GPIOA_K2_20A                11U
#define GPIOA_USB_DP                12U
#define GPIOA_K1_FIRE               12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWDCLK                14U
#define GPIOA_LED_B                 15U
#define GPIOA_K6_20B                15U

#define GPIOB_LED_GREEN             0U
#define GPIOB_ACSW2                 0U
#define GPIOB_DIN4                  1U
#define GPIOB_SW_C12                1U
#define GPIOB_DIN3                  2U
#define GPIOB_SW_C34                2U
#define GPIOB_ODSW4                 3U
#define GPIOB_SW_B3_135             3U
#define GPIOB_ODSW1                 4U
#define GPIOB_ODSW2                 5U
#define GPIOB_I2C1_SCL              6U
#define GPIOB_I2C1_SDA              7U
#define GPIOB_CANR                  8U
#define GPIOB_CAND                  9U
#define GPIOB_DIN2                  10U
#define GPIOB_SW_C56                10U
#define GPIOB_ACSW1                 11U
#define GPIOB_SENSE                 11U
#define GPIOB_SW_UKN                12U
#define GPIOB_K3_21                 13U
#define GPIOB_SW_B0_135             14U
#define GPIOB_K5_S3                 15U

#define GPIOC_PIN0                  0U
#define GPIOC_PIN1                  1U
#define GPIOC_PIN2                  2U
#define GPIOC_PIN3                  3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_PIN7                  7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_PIN10                 10U
#define GPIOC_PIN11                 11U
#define GPIOC_PIN12                 12U
#define GPIOC_CAN_SUSPEND           13U
#define GPIOC_SW_B0_246             14U
#define GPIOC_SW_B1_246             15U

#define GPIOD_PIN0                  0U
#define GPIOD_PIN1                  1U
#define GPIOD_PIN2                  2U
#define GPIOD_PIN3                  3U
#define GPIOD_PIN4                  4U
#define GPIOD_PIN5                  5U
#define GPIOD_PIN6                  6U
#define GPIOD_PIN7                  7U
#define GPIOD_PIN8                  8U
#define GPIOD_PIN9                  9U
#define GPIOD_PIN10                 10U
#define GPIOD_PIN11                 11U
#define GPIOD_PIN12                 12U
#define GPIOD_PIN13                 13U
#define GPIOD_PIN14                 14U
#define GPIOD_PIN15                 15U

#define GPIOE_PIN0                  0U
#define GPIOE_PIN1                  1U
#define GPIOE_PIN2                  2U
#define GPIOE_PIN3                  3U
#define GPIOE_PIN4                  4U
#define GPIOE_PIN5                  5U
#define GPIOE_PIN6                  6U
#define GPIOE_PIN7                  7U
#define GPIOE_PIN8                  8U
#define GPIOE_PIN9                  9U
#define GPIOE_PIN10                 10U
#define GPIOE_PIN11                 11U
#define GPIOE_PIN12                 12U
#define GPIOE_PIN13                 13U
#define GPIOE_PIN14                 14U
#define GPIOE_PIN15                 15U

#define GPIOF_SW_B2_246             0U
#define GPIOF_SW_B3_246             1U
#define GPIOF_DIN1                  1U
#define GPIOF_PIN2                  2U
#define GPIOF_PIN3                  3U
#define GPIOF_PIN4                  4U
#define GPIOF_PIN5                  5U
#define GPIOF_PIN6                  6U
#define GPIOF_PIN7                  7U
#define GPIOF_PIN8                  8U
#define GPIOF_PIN9                  9U
#define GPIOF_PIN10                 10U
#define GPIOF_PIN11                 11U
#define GPIOF_PIN12                 12U
#define GPIOF_PIN13                 13U
#define GPIOF_PIN14                 14U
#define GPIOF_PIN15                 15U

#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

#define VAL_GPIOA_MODER             (PIN_MODE_OUTPUT(GPIOA_PU_OFF) |        \
                                     PIN_MODE_INPUT(GPIOA_ATS) |         \
                                     PIN_MODE_INPUT(GPIOA_KTS) |     \
                                     PIN_MODE_INPUT(GPIOA_VTS) |     \
                                     PIN_MODE_INPUT(GPIOA_STS) |         \
                                     PIN_MODE_INPUT(GPIOA_FB1) |     \
                                     PIN_MODE_INPUT(GPIOA_FB2) |        \
                                     PIN_MODE_OUTPUT(GPIOA_LED_A) |        \
                                     PIN_MODE_OUTPUT(GPIOA_ODSW3) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_UART1_TX) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_UART1_RX) |         \
                                     PIN_MODE_OUTPUT(GPIOA_K2_20A) |          \
                                     PIN_MODE_OUTPUT(GPIOA_K1_FIRE) |          \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDCLK) |      \
                                     PIN_MODE_OUTPUT(GPIOA_LED_B))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_PU_OFF) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_ATS) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_KTS) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_VTS) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_STS) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_FB1) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_FB2) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOA_LED_A) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOA_ODSW3) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_UART1_TX) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_UART1_RX) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_K2_20A) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_K1_FIRE) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDCLK) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_LED_B))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOA_PU_OFF) |        \
                                     PIN_OSPEED_VERYLOW(GPIOA_ATS) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_KTS) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_VTS) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_STS) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_FB1) |  \
                                     PIN_OSPEED_VERYLOW(GPIOA_FB2) |    \
                                     PIN_OSPEED_VERYLOW(GPIOA_LED_A) |    \
                                     PIN_OSPEED_VERYLOW(GPIOA_ODSW3) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_UART1_TX) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_UART1_RX) |     \
                                     PIN_OSPEED_VERYLOW(GPIOA_K2_20A) |      \
                                     PIN_OSPEED_VERYLOW(GPIOA_K1_FIRE) |      \
                                     PIN_OSPEED_VERYLOW(GPIOA_SWDIO) |      \
                                     PIN_OSPEED_VERYLOW(GPIOA_SWDCLK) |      \
                                     PIN_OSPEED_VERYLOW(GPIOA_LED_B))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_PU_OFF) |        \
                                     PIN_PUPDR_PULLDOWN(GPIOA_ATS) |         \
                                     PIN_PUPDR_PULLDOWN(GPIOA_KTS) |     \
                                     PIN_PUPDR_PULLDOWN(GPIOA_VTS) |     \
                                     PIN_PUPDR_PULLDOWN(GPIOA_STS) |         \
                                     PIN_PUPDR_PULLDOWN(GPIOA_FB1) |     \
                                     PIN_PUPDR_PULLDOWN(GPIOA_FB2) |        \
                                     PIN_PUPDR_FLOATING(GPIOA_LED_A) |        \
                                     PIN_PUPDR_FLOATING(GPIOA_ODSW3) |         \
                                     PIN_PUPDR_FLOATING(GPIOA_UART1_TX) |         \
                                     PIN_PUPDR_PULLUP(GPIOA_UART1_RX) |         \
                                     PIN_PUPDR_FLOATING(GPIOA_K2_20A) |          \
                                     PIN_PUPDR_FLOATING(GPIOA_K1_FIRE) |          \
                                     PIN_PUPDR_PULLUP(GPIOA_SWDIO) |      \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWDCLK) |      \
                                     PIN_PUPDR_FLOATING(GPIOA_LED_B))
#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_PU_OFF) |        \
                                     PIN_ODR_HIGH(GPIOA_ATS) |         \
                                     PIN_ODR_HIGH(GPIOA_KTS) |     \
                                     PIN_ODR_HIGH(GPIOA_VTS) |     \
                                     PIN_ODR_HIGH(GPIOA_STS) |         \
                                     PIN_ODR_HIGH(GPIOA_FB1) |     \
                                     PIN_ODR_HIGH(GPIOA_FB2) |        \
                                     PIN_ODR_LOW(GPIOA_LED_A) |        \
                                     PIN_ODR_LOW(GPIOA_ODSW3) |         \
                                     PIN_ODR_HIGH(GPIOA_UART1_TX) |         \
                                     PIN_ODR_HIGH(GPIOA_UART1_RX) |         \
                                     PIN_ODR_HIGH(GPIOA_K2_20A) |          \
                                     PIN_ODR_HIGH(GPIOA_K1_FIRE) |          \
                                     PIN_ODR_HIGH(GPIOA_SWDIO) |      \
                                     PIN_ODR_HIGH(GPIOA_SWDCLK) |      \
                                     PIN_ODR_LOW(GPIOA_LED_B))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_PU_OFF, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_ATS, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_KTS, 0U) |     \
                                     PIN_AFIO_AF(GPIOA_VTS, 0U) |     \
                                     PIN_AFIO_AF(GPIOA_STS, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_FB1, 0U) |     \
                                     PIN_AFIO_AF(GPIOA_FB2, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_LED_A, 0U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_ODSW3, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_UART1_TX, 1U) |         \
                                     PIN_AFIO_AF(GPIOA_UART1_RX, 1U) |         \
                                     PIN_AFIO_AF(GPIOA_K2_20A, 0U) |          \
                                     PIN_AFIO_AF(GPIOA_K1_FIRE, 0U) |          \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |      \
                                     PIN_AFIO_AF(GPIOA_SWDCLK, 0U) |      \
                                     PIN_AFIO_AF(GPIOA_LED_B, 0U))

/*
 * GPIOB setup:
 *
 */
#define VAL_GPIOB_MODER             (PIN_MODE_OUTPUT(GPIOB_LED_GREEN) |         \
                                     PIN_MODE_INPUT(GPIOB_DIN4) |           \
                                     PIN_MODE_INPUT(GPIOB_DIN3) |           \
                                     PIN_MODE_OUTPUT(GPIOB_ODSW4) |        \
                                     PIN_MODE_OUTPUT(GPIOB_ODSW1) |         \
                                     PIN_MODE_OUTPUT(GPIOB_ODSW2) |         \
                                     PIN_MODE_ALTERNATE(GPIOB_I2C1_SCL) |        \
                                     PIN_MODE_ALTERNATE(GPIOB_I2C1_SDA) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_CANR) |        \
                                     PIN_MODE_ALTERNATE(GPIOB_CAND) |        \
                                     PIN_MODE_INPUT(GPIOB_DIN2) |         \
                                     PIN_MODE_OUTPUT(GPIOB_ACSW1) |          \
                                     PIN_MODE_OUTPUT(GPIOB_SW_UKN) |          \
                                     PIN_MODE_OUTPUT(GPIOB_K3_21) |          \
                                     PIN_MODE_OUTPUT(GPIOB_SW_B0_135) |          \
                                     PIN_MODE_OUTPUT(GPIOB_K5_S3))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_LED_GREEN) |         \
                                     PIN_OTYPE_PUSHPULL(GPIOB_DIN4) |           \
                                     PIN_OTYPE_PUSHPULL(GPIOB_DIN3) |           \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ODSW4) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ODSW1) |         \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ODSW2) |         \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_I2C1_SCL) |        \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_I2C1_SDA) |           \
                                     PIN_OTYPE_PUSHPULL(GPIOB_CANR) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOB_CAND) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOB_DIN2) |         \
                                     PIN_OTYPE_PUSHPULL(GPIOB_ACSW1) |          \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SW_UKN) |          \
                                     PIN_OTYPE_PUSHPULL(GPIOB_K3_21) |          \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SW_B0_135) |          \
                                     PIN_OTYPE_PUSHPULL(GPIOB_K5_S3))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOB_LED_GREEN) |         \
                                     PIN_OSPEED_VERYLOW(GPIOB_DIN4) |           \
                                     PIN_OSPEED_VERYLOW(GPIOB_DIN3) |           \
                                     PIN_OSPEED_VERYLOW(GPIOB_ODSW4) |        \
                                     PIN_OSPEED_VERYLOW(GPIOB_ODSW1) |         \
                                     PIN_OSPEED_VERYLOW(GPIOB_ODSW2) |         \
                                     PIN_OSPEED_VERYLOW(GPIOB_I2C1_SCL) |        \
                                     PIN_OSPEED_VERYLOW(GPIOB_I2C1_SDA) |           \
                                     PIN_OSPEED_VERYLOW(GPIOB_CANR) |        \
                                     PIN_OSPEED_VERYLOW(GPIOB_CAND) |        \
                                     PIN_OSPEED_VERYLOW(GPIOB_DIN2) |         \
                                     PIN_OSPEED_VERYLOW(GPIOB_ACSW1) |          \
                                     PIN_OSPEED_VERYLOW(GPIOB_SW_UKN) |          \
                                     PIN_OSPEED_VERYLOW(GPIOB_K3_21) |          \
                                     PIN_OSPEED_VERYLOW(GPIOB_SW_B0_135) |          \
                                     PIN_OSPEED_VERYLOW(GPIOB_K5_S3))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_LED_GREEN) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_DIN4) |           \
                                     PIN_PUPDR_PULLUP(GPIOB_DIN3) |           \
                                     PIN_PUPDR_FLOATING(GPIOB_ODSW4) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_ODSW1) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_ODSW2) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_I2C1_SCL) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_I2C1_SDA) |           \
                                     PIN_PUPDR_FLOATING(GPIOB_CANR) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_CAND) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_DIN2) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_ACSW1) |          \
                                     PIN_PUPDR_FLOATING(GPIOB_SW_UKN) |          \
                                     PIN_PUPDR_FLOATING(GPIOB_K3_21) |          \
                                     PIN_PUPDR_FLOATING(GPIOB_SW_B0_135) |          \
                                     PIN_PUPDR_FLOATING(GPIOB_K5_S3))
#define VAL_GPIOB_ODR               (PIN_ODR_LOW(GPIOB_LED_GREEN) |         \
                                     PIN_ODR_LOW(GPIOB_DIN4) |           \
                                     PIN_ODR_LOW(GPIOB_DIN3) |           \
                                     PIN_ODR_LOW(GPIOB_ODSW4) |        \
                                     PIN_ODR_LOW(GPIOB_ODSW1) |         \
                                     PIN_ODR_LOW(GPIOB_ODSW2) |         \
                                     PIN_ODR_HIGH(GPIOB_I2C1_SCL) |        \
                                     PIN_ODR_LOW(GPIOB_I2C1_SDA) |           \
                                     PIN_ODR_LOW(GPIOB_CANR) |        \
                                     PIN_ODR_LOW(GPIOB_CAND) |        \
                                     PIN_ODR_LOW(GPIOB_DIN2) |         \
                                     PIN_ODR_LOW(GPIOB_ACSW1) |          \
                                     PIN_ODR_LOW(GPIOB_SW_UKN) |          \
                                     PIN_ODR_LOW(GPIOB_K3_21) |          \
                                     PIN_ODR_LOW(GPIOB_SW_B0_135) |          \
                                     PIN_ODR_LOW(GPIOB_K5_S3))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_LED_GREEN, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_DIN4, 0U) |           \
                                     PIN_AFIO_AF(GPIOB_DIN3, 0U) |           \
                                     PIN_AFIO_AF(GPIOB_ODSW4, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_ODSW1, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_ODSW2, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_I2C1_SCL, 1U) |        \
                                     PIN_AFIO_AF(GPIOB_I2C1_SDA, 1U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_CANR, 4U) |        \
                                     PIN_AFIO_AF(GPIOB_CAND, 4U) |        \
                                     PIN_AFIO_AF(GPIOB_DIN2, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_ACSW1, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_SW_UKN, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_K3_21, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_SW_B0_135, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_K5_S3, 0U))

/*
 * GPIOC setup:
 *
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |         \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |         \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |         \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN12) |          \
                                     PIN_MODE_OUTPUT(GPIOC_CAN_SUSPEND) |         \
                                     PIN_MODE_OUTPUT(GPIOC_SW_B0_246) |       \
                                     PIN_MODE_OUTPUT(GPIOC_SW_B1_246))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_CAN_SUSPEND) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SW_B0_246) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SW_B1_246))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOC_PIN0) |        \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN1) |        \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN2) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN3) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN4) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN5) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN6) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN7) |        \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN8) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN9) |          \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN10) |         \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN11) |         \
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN12) |         \
                                     PIN_OSPEED_VERYLOW(GPIOC_CAN_SUSPEND) |        \
                                     PIN_OSPEED_VERYLOW(GPIOC_SW_B0_246) |      \
                                     PIN_OSPEED_VERYLOW(GPIOC_SW_B1_246))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_PIN0) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN1) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN7) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN12) |        \
                                     PIN_PUPDR_FLOATING(GPIOC_CAN_SUSPEND) |     \
                                     PIN_PUPDR_FLOATING(GPIOC_SW_B0_246) |   \
                                     PIN_PUPDR_FLOATING(GPIOC_SW_B1_246))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN0) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN1) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN7) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOC_CAN_SUSPEND) |           \
                                     PIN_ODR_LOW(GPIOC_SW_B0_246) |         \
                                     PIN_ODR_LOW(GPIOC_SW_B1_246))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_CAN_SUSPEND, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_SW_B0_246, 0U) |      \
                                     PIN_AFIO_AF(GPIOC_SW_B1_246, 0U))

/*
 * GPIOD setup:
 *
 * PD0  - PIN0                      (input pullup).
 * PD1  - PIN1                      (input pullup).
 * PD2  - PIN2                      (input pullup).
 * PD3  - PIN3                      (input pullup).
 * PD4  - PIN4                      (input pullup).
 * PD5  - PIN5                      (input pullup).
 * PD6  - PIN6                      (input pullup).
 * PD7  - PIN7                      (input pullup).
 * PD8  - PIN8                      (input pullup).
 * PD9  - PIN9                      (input pullup).
 * PD10 - PIN10                     (input pullup).
 * PD11 - PIN11                     (input pullup).
 * PD12 - PIN12                     (input pullup).
 * PD13 - PIN13                     (input pullup).
 * PD14 - PIN14                     (input pullup).
 * PD15 - PIN15                     (input pullup).
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN15))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN15))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOD_PIN0) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN1) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN2) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN3) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN4) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN5) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN6) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN7) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN8) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN9) |          \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN10) |         \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN11) |         \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN12) |         \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN13) |         \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN14) |         \
                                     PIN_OSPEED_VERYLOW(GPIOD_PIN15))
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP(GPIOD_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN15))
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN15))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN7, 0U))
#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN15, 0U))

/*
 * GPIOE setup:
 *
 * PE0  - PIN0                      (input pullup).
 * PE1  - PIN1                      (input pullup).
 * PE2  - PIN2                      (input pullup).
 * PE3  - PIN3                      (input pullup).
 * PE4  - PIN4                      (input pullup).
 * PE5  - PIN5                      (input pullup).
 * PE6  - PIN6                      (input pullup).
 * PE7  - PIN7                      (input pullup).
 * PE8  - PIN8                      (input pullup).
 * PE9  - PIN9                      (input pullup).
 * PE10 - PIN10                     (input pullup).
 * PE11 - PIN11                     (input pullup).
 * PE12 - PIN12                     (input pullup).
 * PE13 - PIN13                     (input pullup).
 * PE14 - PIN14                     (input pullup).
 * PE15 - PIN15                     (input pullup).
 */
#define VAL_GPIOE_MODER             (PIN_MODE_INPUT(GPIOE_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOE_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN15))
#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN15))
#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOE_PIN0) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN1) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN2) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN3) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN4) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN5) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN6) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN7) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN8) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN9) |          \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN10) |         \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN11) |         \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN12) |         \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN13) |         \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN14) |         \
                                     PIN_OSPEED_VERYLOW(GPIOE_PIN15))
#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP(GPIOE_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN15))
#define VAL_GPIOE_ODR               (PIN_ODR_HIGH(GPIOE_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN15))
#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIOE_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN7, 0U))
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOE_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN15, 0U))

/*
 * GPIOF setup:
 *
 */
#define VAL_GPIOF_MODER             (PIN_MODE_OUTPUT(GPIOF_SW_B2_246) |         \
                                     PIN_MODE_OUTPUT(GPIOF_SW_B3_246) |        \
                                     PIN_MODE_INPUT(GPIOF_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN15))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_SW_B2_246) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_SW_B3_246) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN15))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOF_SW_B2_246) |        \
                                     PIN_OSPEED_VERYLOW(GPIOF_SW_B3_246) |       \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN2) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN3) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN4) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN5) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN6) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN7) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN8) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN9) |          \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN10) |         \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN11) |         \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN12) |         \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN13) |         \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN14) |         \
                                     PIN_OSPEED_VERYLOW(GPIOF_PIN15))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_FLOATING(GPIOF_SW_B2_246) |     \
                                     PIN_PUPDR_FLOATING(GPIOF_SW_B3_246) |    \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN15))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_SW_B2_246) |           \
                                     PIN_ODR_HIGH(GPIOF_SW_B3_246) |          \
                                     PIN_ODR_HIGH(GPIOF_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_SW_B2_246, 0U) |        \
                                     PIN_AFIO_AF(GPIOF_SW_B3_246, 0U) |       \
                                     PIN_AFIO_AF(GPIOF_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0U))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN15, 0U))

void boardInit(void);

#endif /* BOARD_BOARD_H_ */
