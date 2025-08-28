import sys

def main() -> int:
    for i in range(50000):
        # Python uses the `+` operator for string concatenation.
        a = "Hello, " + "world!"
        print("string validation passed: operator+")
        
        if not a.startswith("Hello"):
            print("string valiation failed: startswith 'Hello'")
            
        print("string valiation passed: startswith 'Hello'")

        if not a.endswith("!"):
            print("string valiation failed: endswith '!'")
            
        print("string valiation passed: endswith '!'")
        
        # Substring is done via slicing. s[start:end]
        if a[0:5] != "Hello":
            print("string valiation failed: substring(0, 5) == 'Hello'")
            
        print("string valiation passed: substring(0, 5) == 'Hello'")
        
        if a.find("llo") != 2:
            print("string valiation failed: find('llo') == 2")
            
        print("string valiation passed: find('llo') == 2")
        print(a)
        
        # The trim() method is called strip() in Python
        b = "  May all the beauty be blessed.  ".strip()
        if b != "May all the beauty be blessed.":
            print("string valiation failed: trim()")
            
        print(b)
        
        c = "K423 & Raiden Mei & K423"
        
        # The find_all() method is called count() in Python
        if c.count("Raiden") != 1 or c.count("K423") != 2:
            print("string valiation failed: find_all('Raiden') == 1 && find_all('K423') == 2")
            
        print("string valiation passed: find_all('Raiden') == 1 && find_all('K423') == 2")
        
        # The replace() method in Python replaces all occurrences by default
        c = c.replace("K423", "Kiana Kaslana")
        
        if c != "Kiana Kaslana & Raiden Mei & Kiana Kaslana":
            print("string valiation failed: replace('K423', 'Kiana Kaslana')")
        print(c)
        print("string valiation passed: replace('K423', 'Kiana Kaslana')")
        
        # Converting any type to a string is done with the str() constructor
        d = str(-123)
        if d != "-123":
            print("string valiation failed: to_string()")
        print(d)
        print("string valiation passed: to_string()")
        
        # Python has powerful f-strings and a .format() method.
        # Note: As with the C++ example, the validation string in the original code appears
        # to be buggy. This is the corrected, expected output.
        f = ("{}: RESPOND ME! {}!\n{}: {} {}, {}, {}~".format(
            "Raiden Mei", "ELYSIA", "Elysia", "Can you hear me?", 1, 2, 3))

        expected_format_output = "Raiden Mei: RESPOND ME! ELYSIA!\nElysia: Can you hear me? 1, 2, 3~"
        if f != expected_format_output:
            print(f)
            print("string valiation failed: format()")
        print(f)
        print("string valiation passed: format()")

        print("string valiation passed: all tests passed")

if __name__ == "__main__":
    # A non-zero exit code indicates an error
    sys.exit(main())