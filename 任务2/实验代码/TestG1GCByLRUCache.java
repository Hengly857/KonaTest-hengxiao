/*
 * @test
 * @bug 1234567
 * @summary implementa LRU Cache with Whitebox API to monitor old age region
 * @library /test/lib
 * @modules java.base/jdk.internal.misc
 *          java.management
 * @build jdk.test.whitebox.WhiteBox
 * @run driver jdk.test.lib.helpers.ClassFileInstaller jdk.test.whitebox.WhiteBox
 * @run main/othervm -Xbootclasspath/a:. -XX:+UnlockDiagnosticVMOptions -XX:+WhiteBoxAPI -XX:+UseG1GC -Xlog:gc* -Xlog:gc:g1gc.log  TestG1GCByLRUCache
 */

import jdk.test.whitebox.WhiteBox;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryMXBean;
import java.lang.management.MemoryUsage;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;
 
public class TestG1GCByLRUCache {
 
    private static final WhiteBox WB = WhiteBox.getWhiteBox();
    private static final int M = 1024 * 1024;
    private static final int K = 1024;
    
    public static void main(String[] args) {
        MemoryMXBean memoryMXBean = ManagementFactory.getMemoryMXBean();
        MemoryUsage heapMemoryUsage = memoryMXBean.getHeapMemoryUsage();
        int maxHeapSize = (int)(heapMemoryUsage.getMax() / M);
        System.out.printf("Max heap size: %d MB\n", maxHeapSize);
        
        //LRU CACHE SIZE
        int cacheSize = (int) (heapMemoryUsage.getMax() * 0.8 / K);
        //TOTAL OPERATIONS
        int totalOperations = (int) Math.round(heapMemoryUsage.getMax() * 1.1 / K);
        System.out.println("Cache size:" + cacheSize);
        System.out.println("Total operations:" + totalOperations);
        
        LRUCache<Integer, byte[]> cache = new LRUCache<>(cacheSize);
        Map<Integer, byte[]> ifObjectsInOldAge = new HashMap<>();
        Random random = new Random();

        for (int i = 0; i < totalOperations; i++) {
            int key = random.nextInt(cacheSize);
            byte[] value = new byte[K];
            cache.put(key, value);
            ifObjectsInOldAge.put(key, value);
            
            //每10万次运行 查看一次老年代region中存活对象情况
            if (i % 100000 == 0) {
                System.out.println("Operations: " + i);
                int liveObjects = countLiveObjectsInOldGen(ifObjectsInOldAge);
                System.out.println("Live objects in old region: " + liveObjects);
            
                // 获取混合 GC 信息
                long[] gcInfoBelow50 = WB.g1GetMixedGCInfo(50); // 假设我们想要获取存活率低于 50% 的区域
                System.out.println("old regions with liveness less than 50%");
                System.out.println("Total Old Regions: " + gcInfoBelow50[0] + 
                                   ", Total Memory of Old Regions: " + gcInfoBelow50[1] + 
                                   ", Estimated Memory to Free: " + gcInfoBelow50[2]);

                System.out.println("old regions with liveness less than 70%");
                long[] gcInfoBelow70 = WB.g1GetMixedGCInfo(70); // 假设我们想要获取存活率低于70% 的区域
                System.out.println("Total Old Regions: " + gcInfoBelow70[0] + 
                                   ", Total Memory of Old Regions: " + gcInfoBelow70[1] + 
                                   ", Estimated Memory to Free: " + gcInfoBelow70[2]);

                System.out.println("old regions with liveness less than 90%");
                long[] gcInfoBelow90 = WB.g1GetMixedGCInfo(90); // 假设我们想要获取存活率低于 90% 的区域
                System.out.println("Total Old Regions: " + gcInfoBelow90[0] + 
                                   ", Total Memory of Old Regions: " + gcInfoBelow90[1] + 
                                   ", Estimated Memory to Free: " + gcInfoBelow90[2]);
                                   
                System.gc(); // 手动触发垃圾回收
            }
        }

        // 最终检查
        System.out.println("Final check:");
        int finalLiveObjects = countLiveObjectsInOldGen(ifObjectsInOldAge);
        System.out.println("Live objects in old region: " + finalLiveObjects);
    }

    private static int countLiveObjectsInOldGen(Map<Integer, byte[]> ifObjectsInOldAge) {
        int liveObjects = 0;

        // 检查追踪对象是否在老年代中
        for (Map.Entry<Integer, byte[]> entry : ifObjectsInOldAge.entrySet()) {
            if (WB.isObjectInOldGen(entry.getValue())) {
                liveObjects++;
            }
        }

        return liveObjects;
    }


    private static class LRUCache<K, V> extends LinkedHashMap<K, V> {
        private final int capacity;
 
        public LRUCache(int capacity) {
            super(capacity, 0.75f, true);
            this.capacity = capacity;
        }
 
        @Override
        protected boolean removeEldestEntry(Map.Entry<K, V> eldest) {
            return size() > capacity;
        }
    }
}
