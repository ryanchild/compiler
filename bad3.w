integer function main()
  integer i;
  integer r
  boolean bool;

  integer arr[5];

  integer function callme(integer a[10], boolean b)
  begin
    callme := 0;
  end function;

begin
  i := callme(arr, bool);

  main := 0;
end function
