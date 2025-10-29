public class StringBenchmark {


    public static void main(String[] args) {
        for (int i = 0; i < 50000; i++) {
            String a = "Hello, " + "world!";
            System.out.println("string validation passed: operator+");


            if (!a.startsWith("Hello")) {
                System.out.println("string valiation failed: startswith 'Hello'");
            }
            System.out.println("string valiation passed: startswith 'Hello'");


            if (!a.endsWith("!")) {
                System.out.println("string valiation failed: endswith '!'");
            }
            System.out.println("string valiation passed: endswith '!'");


            if (!a.substring(0, 5).equals("Hello")) {
                System.out.println("string valiation failed: substring(0, 5) == 'Hello'");
            }
            System.out.println("string valiation passed: substring(0, 5) == 'Hello'");


            if (a.indexOf("llo") != 2) {
                System.out.println("string valiation failed: find('llo') == 2");
            }
            System.out.println("string valiation passed: find('llo') == 2");
            System.out.println(a);


            String b = "  May all the beauty be blessed.  ".trim();
            if (!b.equals("May all the beauty be blessed.")) {
                System.out.println("string valiation failed: trim()");
            }
            System.out.println(b);


            String c = "K423 & Raiden Mei & K423";


            if (countOccurrences(c, "Raiden") != 1 || countOccurrences(c, "K423") != 2) {
                System.out.println("string valiation failed: find_all('Raiden') == 1 && find_all('K423') == 2");
            }
            System.out.println("string valiation passed: find_all('Raiden') == 1 && find_all('K423') == 2");


            c = c.replace("K423", "Kiana Kaslana");


            if (!c.equals("Kiana Kaslana & Raiden Mei & Kiana Kaslana")) {
                System.out.println("string valiation failed: replace('K423', 'Kiana Kaslana')");
            }
            System.out.println(c);
            System.out.println("string valiation passed: replace('K423', 'Kiana Kaslana')");


            String d = Integer.toString(-123);
            if (!d.equals("-123")) {
                System.out.println("string valiation failed: to_string()");
            }
            System.out.println(d);
            System.out.println("string valiation passed: to_string()");


            String f = String.format(
                    "%s: RESPOND ME! %s!\n%s: %s %d, %d, %d~",
                    "Raiden Mei", "ELYSIA", "Elysia", "Can you hear me? 1, 2, 3~", 1, 2, 3);


            if (!f.equals("Raiden Mei: RESPOND ME! ELYSIA!\nElysia: Can you hear me? 1, 2, 3~ 1, 2, 3~")) {
                System.out.println(f);
                System.out.println("string valiation failed: format()");
            }
            System.out.println(f);


            System.out.println("string valiation passed: format()");


            System.out.println("string valiation passed: all tests passed");
        }
    }


    public static int countOccurrences(String str, String subStr) {
        int count = 0;
        int lastIndex = 0;
        while (lastIndex != -1) {
            lastIndex = str.indexOf(subStr, lastIndex);
            if (lastIndex != -1) {
                count++;
                lastIndex += subStr.length();
            }
        }
        return count;
    }
}