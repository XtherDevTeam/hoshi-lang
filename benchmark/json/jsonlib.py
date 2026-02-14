import json

def iteration(s: str):
    # json.loads is the Python equivalent of json.parse
    json.loads(s)

def main():
    try:
        # Open the file for reading
        with open("benchmark/json/test.json", "r") as f:
            s = f.read()
        
        # Print the string and a newline
        print(s)

        # Loop 1,000,000 times
        for _ in range(1000000):
            iteration(s)
            
        return 0
    except FileNotFoundError:
        print("Error: benchmark/json/test.json not found.")
        return 1
    except Exception as e:
        print(f"An error occurred: {e}")
        return 1

if __name__ == "__main__":
    main()