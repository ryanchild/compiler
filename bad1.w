integer function main()
  integer i;

  integer function callme(integer a, boolean b)
  begin
    callme := 0;
  end function;

begin
  i := callme(42);

end function
