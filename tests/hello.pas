program Test;

const ask = "What's your name ?\n";
var person : record 
  name : array [0..32] of Char;
  age : Int;
end;

begin
  write(ask);
  read(person.name);
  repeat 
    write("What's your age ? (1-120)\n");
    read(person.age)
  until (person.age >= 1) and (person.age <= 120);
  write("Hello ",person.age,"-old ",person.name," !\n")
end.