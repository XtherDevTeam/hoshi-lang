public class FloatingPointBenchmark {


    public static void main(String[] args) {
        double x = 1.0;
        for (int i = 0; i < 99999999; i++) {
            x = (i + i + 2 * i + 1 - 0.379) / x;
        }
        System.out.println(x);
    }
}