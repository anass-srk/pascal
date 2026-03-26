program the_calculator;

var
  str: array [0..255] of Char;
  index: Int;
  current: Char;
  c2i: array ['0'..'9'] of Int; {No conversion available yet}

function add : Int;.
function get_int : Int;.

procedure next;
begin
  while (str[index] <> '\0') and ((str[index] = ' ') or (str[index] = '\n') or (str[index] = '\t')) do
    index := index + 1;
  current := str[index]
end.

function factor : Int;
begin
  factor := 0;
  if current <> '\0' then
    begin
      if current = '-' then
        begin
          index := index + 1;
          next();
          factor := factor() * (-1)
        end
      else 
      if current = '(' then
        begin
          index := index + 1;
          next();
          factor := add();
          index := index + 1;
          next()
        end
      else
        factor := get_int();
        next()
    end
end.

function mult : Int;
var continue : Bool;
begin
  continue := true;
  mult := factor();
  while (current <> '\0') and continue do
    begin
      if current = '*' then
        begin
          index := index + 1;
          next();
          mult := mult * factor()
        end
      else
      if current = '/' then
        begin
          index := index + 1;
          next();
          mult := mult / factor()
        end
      else
        continue := false
    end
end.

function add : Int;
var continue : Bool;
begin
  continue := true;
  add := mult();
  while (current <> '\0') and continue do
    begin
      if current = '+' then
        begin
          index := index + 1;
          next();
          add := add + mult()
        end
      else
      if current = '-' then
        begin
          index := index + 1;
          next();
          add := add - mult()
        end
      else
        continue := false
    end
end.

function get_int : Int;
  begin
    get_int := 0;
    while (current <> '\0') and (current >= '0') and (current <= '9') do
      begin
        get_int := get_int * 10 + c2i[current];
        index := index + 1;
        next()
      end
  end.

begin
  c2i['0'] := 0;
  c2i['1'] := 1;
  c2i['2'] := 2;
  c2i['3'] := 3;
  c2i['4'] := 4;
  c2i['5'] := 5;
  c2i['6'] := 6;
  c2i['7'] := 7;
  c2i['8'] := 8;
  c2i['9'] := 9;
  index := 0;
  write("Write a mathematical expression (Integers only) :\n");
  read(str);
  next();
  write(add())
end.