README.txt

COP 3402: Systems Software
Homework #4 (PL/0 Compiler)
Due: July 21st, 2024

Authors:
- Devon Villalona
- Izaac Plambeck

Description:
This program implements a PL/0 compiler which performs lexical analysis, parsing, and code generation to produce executable code for a Virtual Machine (VM).

Compilation Instructions:
1. Ensure you have a C compiler installed (e.g., gcc).
2. Open a terminal/command prompt.
3. Navigate to the directory containing the source files.
4. Compile the source code using the following command:
   gcc -Wall -o hw4compiler hw4compiler.c

Usage Instructions:
1. After compiling, you can run the compiler using the following command:
   ./hw4compiler <source file>

   Example:
   ./hw4compiler input1.txt

2. The program will generate the following output files:
   - lexOutput.txt: Contains the output of the lexical analysis.
   - errorOutput.txt: Contains any error messages encountered during compilation.
   - output.txt: Contains the generated code in a human-readable format.
   - elf.txt: Contains the generated code in a format suitable for the VM.

Test Cases:
The following input files are provided to test the compiler:

1. input1.txt
   Source Code:
   var x;
   begin
     x := 2;
     write x
   end.
   Expected Output:
   elf1.txt

2. input2.txt
   Source Code:
   var x, y;
   begin
     x := 2;
     y := x + 3;
     write y
   end.
   Expected Output:
   elf2.txt

3. input3.txt
   Source Code:
   var x, y;
   begin
     read x;
     read y;
     if x > y then
       write x
     else
       write y
     fi
   end.
   Expected Output:
   elf3.txt

4. input4.txt
   Source Code:
   const z = 5;
   var x, y;
   begin
     x := z;
     y := x + 2;
     write y
   end.
   Expected Output:
   elf4.txt

5. input5.txt
   Source Code:
   procedure factorial;
   var n, f;
   begin
     f := 1;
     while n > 1 do
       f := f * n;
       n := n - 1
     end;
     write f
   end;
   var x;
   begin
     read x;
     n := x;
     call factorial
   end.
   Expected Output:
   elf5.txt

6. input6.txt
   Source Code:
   var x;
   procedure P;
   begin
     write x
   end;
   begin
     x := 5;
     call P
   end.
   Expected Output:
   elf6.txt

7. input7.txt
   Source Code:
   const max = 10;
   var i;
   begin
     i := 0;
     while i < max do
       i := i + 1;
       write i
     end
   end.
   Expected Output:
   elf7.txt

8. input8.txt
   Source Code:
   var x, y;
   begin
     read x;
     read y;
     if x = y then
       write 1
     else
       write 0
     fi
   end.
   Expected Output:
   elf8.txt

9. input9.txt
   Source Code:
   var a, b;
   procedure add;
   begin
     b := a + b
   end;
   begin
     read a;
     read b;
     call add;
     write b
   end.
   Expected Output:
   elf9.txt

10. input10.txt
    Source Code:
    var x;
    procedure double;
    begin
      x := x * 2
    end;
    begin
      read x;
      call double;
      write x
    end.
    Expected Output:
    elf10.txt

Instructions for Running Test Cases:
1. Place the provided input files (input1.txt, input2.txt, ..., input10.txt) in the same directory as the compiled compiler (hw4compiler).
2. Run each test case using the following command:
   ./hw4compiler input1.txt
   ./hw4compiler input2.txt
   ...
   ./hw4compiler input10.txt

3. Verify the generated elf.txt file against the expected output (elf1.txt, elf2.txt, ..., elf10.txt).



Submission Contents:
- hw4compiler.c: Source code of the PL/0 compiler.
- README.txt: This file.
- lexOutput.txt: Sample lexical output file.
- errorOutput.txt: Sample error output file.
- output.txt: Sample generated code output file.
- elf.txt: Sample executable code file for the VM.
- Test cases: input1.txt, input2.txt, ..., input10.txt.
- Expected outputs: elf1.txt, elf2.txt, ..., elf10.txt.

