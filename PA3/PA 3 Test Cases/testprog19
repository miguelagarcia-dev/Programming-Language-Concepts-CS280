program circle;
const
	pi = 3.14;
var
	{Clean program testing then-clause nested if statement}
	r, a, p, b : real; 
	flag : boolean := true;
	i, j : integer := 0;
	str : string := 'End of Program';	
begin
	p := 0;
	{Reading input from the user 2.5, 5.8, 3}
	readln(b, r, i);
	if (b < 0) or (b > 100) then
	begin
		p := 2 * pi * r ;
		a := r * r * pi;
		writeln ( 'The results of r = ' , r, ', p = ', p, ' and a = ', a)
	end
	else
	begin
	    if (i < 0) then 
			begin r := r + 1; p := 2 * pi * r end
	    else
			begin
				r := r - 1; p := 2 * pi * r; a := r * r * pi;
				writeln ( 'The results of r = ' , r, ', p = ', p, ' and a = ', a);  
			end; 
	    flag := false
	end;
	writeln(str)
end.
                  