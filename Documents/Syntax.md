# The syntax of hoshi-lang

### Identifier

```bnf
Identifier ::= TokIdentifier
```

### StaticMemberAccessExpression

```bnf
StaticMemberAccessExpression ::= Identifier { TokStaticMemberAccessSign FunctionInvokingExpression }
                               | Identifier { TokStaticMemberAccessSign StaticMemberAccessExpression }
```

### Literals

```bnf
Literals ::= TokString | TokInteger | TokDecimal | TokBoolean | TokCharacter
```

### TemplateArguments

```bnf
TemplateArguments ::= TokLessThan { { StaticMemberAccessExpressionWithTemplateArguments TokColon } StaticMemberAccessExpressionWithTemplateArguments } TokMoreThan
```

### StaticMemberAccessExpressionWithTemplateArguments

```bnf
StaticMemberAccessExpressionWithTemplateArguments ::= StaticMemberAccessExpression { TemplateArguments }
```

```bnf
TypeDescriptorParser ::= StaticMemberAccessExpressionWithTemplateArguments
```

### IdentifierWithTypeDescriptor

```bnf
IdentifierWithTypeDescriptor ::= TokIdentifier ":" TypeDescriptorParser 
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
FunctionInvokingExpression ::= TokIdentifierWithTemplateArguments { TokLeftPartheses Arguments TokRightParentheses }
```

### MemberExpression

```bnf
MemberExpression ::= Identifier
                   | Identifier { "." FunctionInvokingExpression }
                   | Identifier { "." MemberExpression }
```

### SingleExpression

```bnf
SingleExpression ::= [TokPlus | TokMinus | TokBinaryNot | TokIncrementSign | TokDecrementSign] (MemberExpression | Literals) 
```

### MultiplicationExpression

```bnf
MultiplicationExpression ::= SingleExpression { (TokAsterisk | TokSlash | PercentSign) MultipleExpression }
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