# Table of Contents (目录)

1. **Introduction (模块概述)**
2. **Core Architecture & Mathematical Design (核心架构与数学设计)**  
2.1 **Time Representation (时间表示法)**  
2.2 **Fixed-point Quantization (定点量化原理)**  
3. **API Reference & Structural Breakdown (核心类与数据结构解析)**  
4. **Implementation & Frame Advanced Logic (时序推进机制)**  
5. **Performance & Concurrency Optimization (性能与并发优化)**  
5.1 **Memory Alignment (内存对齐)**   
5.2 **Lock-free Design (无锁设计)**  
5.3 **Division Elimination (消除除法)**  

---

# Crystal Engine: CRST_Time Module Specification

## 1. Introduction (模块概述)

`CRST_Time` 模块是 Crystal Engine 内部负责全局时序控制与高精度时间推进的核心子系统。在游戏引擎与实时仿真系统中，传统的浮点数时间累加会导致严重的 **Precision Loss (精度漂移)** 与 **Non-determinism (非确定性)**。本模块旨在通过纯整数的 **Fixed-point Quantization (定点量化)** 机制，构建一个稳定、精确且对多线程友好的高精度时钟系统，为逻辑主帧与物理副帧的精确对齐提供底层支撑。

## 2. Core Architecture & Mathematical Design (核心架构与数学设计)

### 2.1 Time Representation (时间表示法)
为了兼顾长时间运行的防溢出需求与微秒级的物理采样精度，`CRST_Time` 放弃了单精度的 `float` 或双精度的 `double` 秒制表达，转而采用基于 **主帧 + 定点浮点表示的副帧** 结构：
* **Major-frame (主帧)**：代表离散的逻辑时间步长（格点），由基准时间步长 $B$（`base_time_step_nano`）决定。
* **Sub-frame (副帧)**：代表两个逻辑主帧之间的一个时间点。以一个32位定点数表示，其决定此时间点位于两主帧之间的哪个具体位置。

### 2.2 Fixed-point Quantization (定点量化原理)
在 `ClockBase` 推进时，若两次更新之间的时间间隔未能凑满一个完整逻辑主帧的基准时间步长 $B$  
（即 $D$ = `accumulator_nano` < `base_time_step_nano`），则该时间间隔会被量化后并入副帧计数。本模块采用 **Rounding to Nearest (最近邻舍入)** 的均匀量化算法。

设物理残余时间为 $D$，基准主帧步长为 $B$，量化精度位宽为 $N = 32$ 位。
量化的最小步长（即固定小数点的最小单位值，LSB） $\Delta$ 为：
$$\Delta = \frac{B}{2^{32}}$$

量化后的 32 位无符号整数值 $Q$（即代码中的 `sub_frac`）的连续域数学表达式为：
$$Q = \text{round}\left(\frac{D}{\Delta}\right) = \left\lfloor \frac{D}{\Delta} + 0.5 \right\rfloor$$

将 $\Delta = \frac{B}{2^{32}}$ 代入上式：
$$Q = \left\lfloor \frac{D \cdot 2^{32}}{B} + 0.5 \right\rfloor = \left\lfloor \frac{D \cdot 2^{32} + \frac{B}{2}}{B} \right\rfloor$$

需要注意的是，过程中 $D \cdot 2^{32}$ 一步可能会导致高位溢出。由于 `accumulator_nano` 在正常工作流中必然小于 `base_time_step_nano`，因此 `scaled` 的最大值约为 $B \cdot 2^{32}$。当 $B$ 超过 $2^{32}$ 纳秒（约 4.29 秒）时，`scaled` 会发生 64 位整型溢出。因此，基准时间步长 $B$ 必须选取在合理的范围内（通常 $\le 1000\text{ms}$），此时全整数安全无溢出。**通过位移操作与扩展类型，上述公式在代码（如 `consumeSubFrame`）中被无损转化为纯整数运算：

```cpp
const CRSTu64 scaled = (accumulator_nano << 32) + (base_time_step_nano / 2);
const CRSTu64 sub_ticks = scaled / base_time_step_nano;
```

当量化结果 `sub_ticks` $\ge 2^{32}$ 时，数学上意味着四舍五入后的残余时间已达到或超越了一个完整的主帧步长，此时可将副帧合并到主帧中。

## 3. API Reference & Structural Breakdown (核心类与数据结构解析)

### 3.1 Duration (时间增量)

`Duration` 用于表征两个时间点之间的相对差值。

* **内存布局**：采用 `alignas(16)` 进行 16 字节内存对齐，确保高性能缓存架构友好。
* **位结构定义**：
* 高 32 位（`bits >> 32`）：主帧计数（Major-frame Count）。
* 低 32 位（`bits & 0xFFFFFFFF`）：副帧定点表示（Sub-frame Fraction）。

### 3.2 TimePoint (时间点)

`TimePoint` 表示绝对时间轴上的位置。

### 3.3 TimerBase (硬件计时器基类)

用于隔离底层操作系统的差异（如 Windows 的 `QueryPerformanceCounter` 与 Linux 的 `clock_gettime`），其唯一职责是通过纯虚接口 `getAbsoluteTimeNano()` 返回外部硬件时间（单位为nanosecond）。

### 3.4 ClockBase (时钟基类)

时钟控制的核心基类，控制引擎内部时序。

* `base_time_step_nano`：基准主帧步长 $B$
* `accumulator_nano`：用于消除计时器抖动的缓冲变量

## 4. Implementation & Frame Advanced Logic (时序推进机制)

```
 [硬件时间采样] ──> advanceTime() ──> 累加至 accumulator_nano
                                                    │
                                                    ▼
 [主帧循环消耗] ──> consumeMajorFrame() ──> 扣减步长，maj_count +1
                                                    │
                                                    ▼
 [副帧残余量化] ──> consumeSubFrame() ──> 量化残余，更新 sub_frac
```

advanceTime() 在每帧帧首调用。 
consumeMajorFrame() 在以 canUpdateFrame() 为条件的 while 循环中调用。 
consumeSubFrame() 在离开主帧消耗循环后、渲染开始前调用，用以确立当前帧的插值权重。 

## 5. Performance & Concurrency Optimization (性能与并发优化)

### 5.1 Memory Alignment (内存对齐)

`Duration` 与 `TimePoint` 均显式声明了 `alignas(16)`。此设计可确保实例在内存中分配时对齐至 16 字节边界，不仅契合现代 CPU 的 Cache Line (缓存行) 填充规则，降低伪共享风险，还为未来引入 SIMD (单指令多数据流) 指令集进行大量时间序列优化预留了空间。

### 5.2 Lock-free Design & Memory Order (无锁设计与内存序)

在多线程渲染引擎中，通常由主线程或物理线程写入时间，而渲染线程和音频线程并发读取时间。此为 SPMC 模型，因此采用如下的内存序设计：  

* **写入侧**：采用 `std::memory_order_release` 内存序。保证在此之前的所有内存写入操作对其他线程可见，防止指令重排。
* **读取侧 (`getCurrentTimePoint`)**：采用 `std::memory_order_acquire` 内存序以确保获取到最新且一致的 `system_time_point`

### 5.3 Division Elimination (消除除法)

在 `consumeSubFrame` 和 `convertToTimePoint` 中存在 64 位无符号除法。通过将 `base_time_step_nano` 在初始化后或编译期作为 `constexpr` 常量传入，可以将其转化为数个周期的快速乘法，从而优化性能。


