import time

class Point:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

def run_benchmark():
    iterations = 10_000_000
    total_sum = 0
    
    start = time.perf_counter()
    
    for i in range(iterations):
        # Heavy heap allocation and dynamic attribute binding
        p = Point(i, 1, 1)
        total_sum += p.x + p.y + p.z
        
    end = time.perf_counter()
    
    print(f"Python (Legacy Class): {end - start:.4f}s")
    print(f"Final Sum: {total_sum}")

if __name__ == "__main__":
    run_benchmark()