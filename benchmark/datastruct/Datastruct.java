class Point {
    long x, y, z;
    Point(long x, long y, long z) {
        this.x = x; this.y = y; this.z = z;
    }
}

public class Datastruct {
    public static void main(String[] args) {
        int iterations = 10_000_000;
        long totalSum = 0;

        long start = System.nanoTime();

        for (int i = 0; i < iterations; i++) {
            // New heap allocation + Object Header + Pointer reference
            Point p = new Point(i, 1, 1);
            totalSum += p.x + p.y + p.z;
        }

        long end = System.nanoTime();
        System.out.println("Java (Heap Object): " + (end - start) / 1e9 + "s");
        System.out.println("Final Sum: " + totalSum);
    }
}