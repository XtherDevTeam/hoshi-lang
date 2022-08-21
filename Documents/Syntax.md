# The syntax of hoshi-lang

### Identifier

```bnf
Identifier ::= TokIdentifier
```

### Literals

```bnf
Literals ::= TokString | TokInteger | TokDecimal | TokBoolean | TokCharacter
```

### IdentifierWithTypeDescriptor

```bnf
IdentifierWithTypeDescriptor ::= TokIdentifier ":" TokIdentifier 
```

### ArgumentsDecl

```bnf
ArgumentsDecl ::= { { IdentifierWithTypeDescriptor TokColon } IdentifierWithTypeDescriptor}
```

### Arguments

```bnf
Arguments ::= { { Expression TokColon } Expression}
```

### FunctionInvokingExpression

```bnf
FunctionInvokingExpression ::= TokIdentifier TokLeftPartheses Arguments TokRightParentheses
```

### MemberExpression

```bnf
MemberExpression ::= Identifier
                   | Identifier "." { FunctionInvokingExpression }
```

### SingleExpression

```bnf
SingleExpression ::= [TokPlus | TokMinus | TokBinaryNot | TokIncrementSign | TokDecrementSign] (MemberExpression | Literals) 
```

### MultipleExpression

```bnf
MultipleExpression ::= SingleExpression { (TokAsterisk | TokSlash | PercentSign) MultipleExpression }
```

### AdditionExpression

```bnf
AdditionExpression ::= MultipleExpression { ( TokPlus | TokMinus ) AdditionExpression } 
```

### BinaryMoveExpression

```bnf
BinaryMoveExpression ::= AdditionExpression { ( TokBinaryLeftMoveSign | TokBinaryRightMoveSign ) BinaryMoveExpression }
```

### LogicComparingExpression

```bnf
LogicComparingExpression ::= BinaryMoveExpression { ( TokMoreThan | TokLessThan | TokMoreThanOrEqual | TokLessThanOrEqual | TokInstanceOf | TokIsImplemented ) LogicComparingExpression }
```

### LogicEqualExpression

```bnf
LogicEqualExpression ::= LogicComparingExpression { ( TokEqual | TokNotEqual ) LogicEqualExpression }
```

### BinaryExpression

```bnf

```