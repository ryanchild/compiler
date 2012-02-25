// test program for compiler

integer function main()
  integer i1; 
  integer i2; 
  integer i3;
  integer i4;
  integer i5;
  integer j;
  integer dump;
  float pi;
  float r;
  float area;
  float volume;
  boolean b1;
  boolean b2;
  string s;

  integer function fibonacci(integer n)
    integer a;
    integer b;
  begin
    if n == 0 then
      fibonacci := 1;
    else
      if n == 1 then
        fibonacci := 1;
      else
        a := fibonacci(n - 1);
        b := fibonacci(n - 2);
        fibonacci := a + b;
      end if;
    end if;
  end function;

  global integer gi;
  boolean function globalTest(integer n)
  begin
    gi := n;
    globalTest := true;
  end function;

  integer arr[5];

  boolean function printArray(integer arr[5], boolean backwards)
    integer i;
    integer dump;
  begin
    for i := 0 i < 5
      if backwards then
        dump := putInt(arr[4 - i]);
      else
        dump := putInt(arr[i]);
      end if;
      dump := putString(" ");
      i := i + 1;
    end for;
    printArray := true;
  end function;

begin

  // global variable test
  dump := putString("Beginning global variable test...\n");
  gi := 0;
  b1 := globalTest(5);
  dump := putString("Global Variable test ");
  if gi == 5 then
    dump := putString("passed");
  else 
    dump := putString("failed");
  end if;

  // array test
  dump := putString("\n\nBeginning array test...\n");  
  arr[0] := 1;
  arr[1] := 2;
  arr[2] := 3;
  arr[3] := 4;
  arr[4] := 5;
  dump := putString("forward: ");
  b1 := printArray(arr, false);
  dump := putString("\nreverse: ");
  b1 := printArray(arr, true);

  // recursion test
  dump := putString("\n\nBeginning recursion test...\n");
  dump := putString("How many fibonacci numbers to print? ");
  i2 := getInt();
  for i1 := 0 i1 < i2
    i1 := i1 + 1;
    dump := putInt(fibonacci(i1));
    dump := putString(" ");
  end for;

  // nested loop test
  dump := putString("\n\nBeginning nested loop test...\n");
  dump := putString("Size of nxn identity matrix to print? ");
  i2 := getInt();
  for i1 := 0 i1 < i2
    for j := 0 j < i2
      if i1 == j then
        dump := putInt(1);
      else
        dump := putInt(0);
      end if;
      dump := putString(" ");
      j := j + 1;
    end for;
    dump := putString("\n");
    i1 := i1 + 1;
  end for;

  // floating point arithmetic test
  dump := putString("\nBeginning floating point arithmetic test...\n");
  dump := putString("Enter radius: ");
  pi := 3.14159265;
  r := getFloat();
  area := pi * r * r;
  volume := (4./3) * pi * r * r * r;
  dump := putString("Area of circle: ");
  dump := putFloat(area);
  dump := putString("\nVolume of sphere: ");
  dump := putFloat(volume);
  
  // bitwise operator test
  dump := putString("\n\nBeginning bitwise operator test...\n");
  dump := putString("Enter an integer: ");
  i1 := getInt();
  dump := putString("Enter another integer: ");
  i2 := getInt();
  i3 := i1 & i2;
  i4 := i1 | i2;
  i5 := not i1;
  dump := putInt(i1);
  dump := putString(" and ");
  dump := putInt(i2);
  dump := putString(": ");
  dump := putInt(i3);
  dump := putString("\n");
  dump := putInt(i1);
  dump := putString(" or ");
  dump := putInt(i2);
  dump := putString(": ");
  dump := putInt(i4);
  dump := putString("\nnot ");
  dump := putInt(i1);
  dump := putString(": ");
  dump := putInt(i5);
  dump := putString("\n");

  // program exit
  main := 0;
end function
