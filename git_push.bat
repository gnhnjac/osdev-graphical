make clean
git add .
for /f "tokens=1-4 delims=/ " %%i in ("%date%") do (
     set dow=%%i
     set month=%%j
     set day=%%k
     set year=%%l
)
set datestr=%day%_%month%_%year%
git commit -m "%datestr%"
git push