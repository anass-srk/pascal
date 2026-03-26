program FibTest;

var i : Int;
const q = "Enter an integer between 0 and 20 :\n";
function fib (n : Int) : Int;
  begin
    if n <= 1 then
      fib := n
    else 
      fib := fib(n-1) + fib(n-2)
  end.
begin
  repeat
    write(q);
    read(i)
  until (i >= 0) and (i <= 20);
  write("fib(",i,") = ",fib(i),'\n')
end.