// test program for compiler

integer function main()
  integer i1; 
  integer i2; 
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

begin

  // recursive test
  dump := putString("How many fibonacci numbers to print? ");
  i2 := getInt();
  for i1 := 0 i1 < i2
    i1 := i1 + 1;
    dump := putInt(fibonacci(i1));
    dump := putString(" ");
  end for;

  // nested loop test
  dump := putString("\n\nSize of nxn identity matrix to print? ");
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
  dump := putString("\nEnter radius: ");
  pi := 3.14159265;
  r := getFloat();
  area := pi * r * r;
  volume := (4./3) * pi * r * r * r;
  dump := putString("Area of circle is: ");
  dump := putFloat(area);
  dump := putString("\nVolume of sphere is: ");
  dump := putFloat(volume);
  dump := putString("\n");
  
  // program exit
  main := 0;
end function
