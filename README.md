# Gesture-Controlled AI Robot: A Distributed STM32 Architecture

[![Board: STM32N6570-DK](https://img.shields.io/badge/Brain-STM32N6570--DK-blue)](https://www.st.com/en/evaluation-tools/stm32n6570-dk.html)
[![Board: STM32F407VGT6](https://img.shields.io/badge/Actuator-STM32F407VGT6-red)](https://www.st.com/en/microcontrollers-microprocessors/stm32f407-417.html)
[![AI: NPU Accelerated](https://img.shields.io/badge/AI-NPU_Accelerated-green)](#)
[![Wireless: Bluetooth HC-05 & HC-06](https://img.shields.io/badge/Wireless-HC--05%20%26%20HC--06-orange)](#)

This project demonstrates a high-performance, real-time robot control system using hand gestures. By leveraging the cutting-edge **NPU (Neural Processing Unit)** of the STM32N6 series and the robust motor control capabilities of the STM32F4, we've built a "Brain-Actuator" distributed system that translates visual human intent into physical motion.

---

## 🚀 Overview

Imagine controlling a mobile robot with nothing but your hand. No joystick, no keyboard—just natural gestures. This project achieves exactly that through a two-stage hardware pipeline:

1.  **The "Brain" (STM32N6570-DK):** Captures high-definition video, runs deep learning models to identify 21 hand landmarks in real-time, and determines the intended command.
2.  **The "Wireless Bridge" (HC-05 & HC-06):** Transmits these commands asynchronously via Bluetooth. The STM32N6 uses an **HC-05 configured as a Master** to send data, which is received by an **HC-06 configured as a Slave** on the robot.
3.  **The "Actuator" (STM32F407VGT6):** Parses the commands and drives 4 DC motors using precisely tuned PWM signals.

---

## 🧠 System Architecture

We opted for a **distributed intelligence** approach to maximize performance and modularity.

### 1. Vision & AI (The Brain)
*   **Sensor:** IMX335 Camera Module.
*   **AI Models:** A sequential execution of two quantized models:
    *   **Palm Detection (PD):** Locates the hand within the frame.
    *   **Hand Landmark (HL):** Identifies 21 key points (knuckles, tips, wrist).
*   **NPU Acceleration:** Inference is handled by the dedicated NPU on the STM32N6, allowing for high FPS and low latency that would be impossible on standard MCUs.
*   **Gesture Logic:** We calculate the relative distance and orientation between the finger tips and the wrist to map hand positions to `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, and `STOP`.

### 2. Motor Control (The Actuator)
*   **Interrupt-Driven Logic:** The STM32F4 uses asynchronous UART reception (`HAL_UART_Receive_IT`) to ensure the CPU is never blocked waiting for commands.
*   **PWM Generation:** TIM3 generates 1kHz PWM signals across 4 channels to control the H-Bridge motor drivers.
*   **Responsive Parsing:** A circular buffer and string parsing (`strstr`) allow for robust command execution even in high-traffic wireless environments.

---

## 🛠️ Hardware Stack

| Component | Role | Description |
| :--- | :--- | :--- |
| **STM32N6570-DK** | Master / AI Unit | NPU-accelerated gesture recognition. |
| **STM32F4-Discovery** | Slave / Control Unit | Motor driving and PWM management. |
| **HC-05 Module** | Transmitter (Master) | Bluetooth SPP link @ 115200 baud, connected to STM32N6. |
| **HC-06 Module** | Receiver (Slave) | Bluetooth SPP link @ 115200 baud, connected to STM32F4. |
| **IMX335** | Vision | 5MP Digital Image Sensor. |
| **4WD Chassis** | Physical | 4 DC Motors with H-Bridge controllers. |

---

## 🔒 Smart Safety Features

Beyond basic movement, the system includes a sophisticated **Locking Mechanism** implemented on the AI side:
*   **State Machine:** The robot starts in a **Locked State (Red LED indication)**.
*   **Unlocking Sequence:** A specific gesture (index finger trigger) is required to unlock the robot.
*   **Auto-Lock:** If the system detects a `STOP` gesture for 200 consecutive frames, it automatically re-locks to prevent accidental movements.

---

## 📂 Project Structure

*   `code robot/`: STM32CubeIDE project for the STM32F4 motor controller.
*   `x-cube-n6-ai-hand-landmarks-main/`: The NPU-accelerated AI application for the STM32N6.


---

## 🎓 About

This project was developed as part of the **S8 Project** at **ENSEA** (Ecole Nationale Supérieure de l'Electronique et de ses Applications).

**Team:**
*   **GARAFI Yassin**
*   **PICQUART Tom**
*   **PENG Haoyu**
*   **MERVEILLE Rose Yomba**

**Supervised by:** M. MONCHAL Laurent (Academic Year 2025/2026)

---

## 📝 License

This project is licensed under the MIT License - see the `LICENSE` file for details.
