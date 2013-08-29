echo 'digraph{';
echo "graph[overlap='prism'];";
echo "node[style=filled];";
echo "graph[splines='curved'];";
echo "graph[rankdir='LR'];";
		
ls .\obj\*.o|%{
	[PSCustomObject]@{
		"file"=$_.Name;
		"symbols"=avr-nm $_ -g|%{
			($address,$type,$name)=$_ -split " +";
			[PSCustomObject]@{
				"addr"=$address;
				"type"=@{
					"U"="undef";
					"T"="code";
					"C"="common";
					"D"="data";
					"W"="weak";
					"V"="wobj";
					"R"="rodata";
					}[$type];
				"name"=$name
				}
			}#|group type
		}
	}|%{
		$cluster = "cluster_{0}" -f $_.file;
		echo ("subgraph '{0}'{1}" -f $cluster,'{');

		echo ("'{0}';" -f $cluster);
		$_.symbols|?{$_.type -ne 'undef'}|%{
			$style = "";
			if($_.type -eq 'code'){$style += "[shape='parallelogram']";}
			if($_.type -eq 'common'){$style += "[shape='house']";}
			if($_.type -eq 'data'){$style += "[shape='trapezium']";}
			echo ("'{0}' {1};" -f $_.name, $style);
			};
			
			
					
		echo "}";
		$_.symbols|?{$_.type -eq 'undef'}|%{echo ("'{0}'->'{1}';" -f $cluster,$_.name)};
		}
echo '}'