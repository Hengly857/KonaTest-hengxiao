# KonaTest-hengxiao<br>
**实验环境**：Linux 5.10.134-17.2.al8.x86_64 (amd64)<br>
**JDK版本**：TencentKona-17.0.11<br>
**Jtreg版本**：jtreg 7.4<br>
**记录参与腾讯KonaJDK开源实践完成三个任务的实验报告**：<br>
- 任务1：写一个测试用例，通过不同的GC参数（Serial GC，Parallel Scavenge，G1GC，ZGC，Shenandoah GC），通过打印GC日志完整的展示GC的各个阶段<br>
- 任务2：专注于G1GC算法，写一个JDK的jtreg测试用例，使用一些现有的whitebox API（有需要的话可以自己扩展whitebox API）来实现一个典型的LRU cache，随机的增加LRU cache内容<br>
- 任务3：利用任务二中的一些数据，分析目前情况下G1GC Adaptive IHOP的原理以及对于mixed GC的影响。根据相关数据，尝试优化一些mixed GC的参数的数值，使其能够根据Java进程的运行状态同样的进行动态调整
