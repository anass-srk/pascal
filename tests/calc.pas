program calculator;

const ask = "Please enter a real number :\n";
const qmsg = "To continue, enter 'c':\n";
var 
  a,b,c : Real;
  input : Char;
function is_op (c : Char) : Bool;
  begin
    is_op := (c = '*') or (c = '/') or (c = '+') or (c = '-')
  end.
begin
  input := 'c';
  while input = 'c' do
    begin
      write(ask);
      read(a);
      write(ask);
      read(b);
      repeat
        write("Enter the operation : ('*', '/', '+' or '-')\n");
        read(input)
      until is_op(input);

      case input of
        '*': c := a * b;
        '/': c := a / b;
        '-': c := a - b;
        '+': c := a + b
        end;
      write(a,' ',input,' ',b," = ",c,'\n');
      write(qmsg);
      read(input)
    end
end.