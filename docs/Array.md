# Hoshi-lang Arrays

Hoshi-lang provides two types of arrays: fixed-size and dynamic.

## 1. Fixed-Size Arrays

Fixed-size arrays have a length that is known at compile time. They are declared with the size specified in square brackets.

### Declaration and Initialization

```hoshi
// An array of 5 integers, initialized with values
let arr1 = int[5](1, 2, 3, 4, 5)

// A 2x3 multi-dimensional array
let arr2 = int[2][3](1, 2, 3, 4, 5, 6)

// An array of 10 integers, initialized with default values (0 for int)
let arr3 = int[10]()
```

### Element Access

Elements are accessed using zero-based indexing.

```hoshi
let first = arr1[0] // first will be 1
arr1[0] = 100      // Modify the first element
```

## 2. Dynamic Arrays

Dynamic arrays is a sort of array which length cannot be predicted in compile-time. They are declared with `new` expression.

### Declaration and Initialization

```hoshi
// A dynamic array with 3 initial elements
let dyn_arr1 = new int[3](1, 2, 3)

// A dynamic array with an initial capacity of 10, all elements default-initialized
let dyn_arr2 = new int[3](10)
```

## 3. The `.length` Property

Both fixed-size and dynamic arrays have a read-only `.length` property that returns the number of elements in the array.

```hoshi
let arr = int[5]()
let len1 = arr.length // len1 will be 5

let dyn_arr = int[](1, 2, 3)
let len2 = dyn_arr.length // len2 will be 3
```
