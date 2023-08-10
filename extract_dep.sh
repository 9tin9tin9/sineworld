#!/bin/sh

echo $1 |
    awk '{
        { if (system("[ -e \"src/" $1 ".c\" ]") == 0) { printf "src/%s.c ", $1 } }
        { if (system("[ -e \"src/" $1 ".h\" ]") == 0) { printf "src/%s.h ", $1 } }
        }' |
    xargs -J % cat % |
    grep -e '#include[[:space:]]*[<"]' |
    sed -E 's/#include//' |
    awk '{
        { F = substr($1,2,length($1)-2) }
        {
            if (substr($1, 1, 1) == "<" && system("[ -e \"lib/" F "\" ]") == 0) { printf "lib/%s\n", F }
            else if (substr($1, 1, 1) == "\"" && system("[ -e \"src/" F "\" ]") == 0) { printf "src/%s\n", F }
        }
        }'
