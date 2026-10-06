@echo off

echo installing phydm commit template
echo ======================

set HOME=d:\phydm_log
if not exist %HOME% (
	echo Warning: can't find %HOME% and create %HOME% folder
	mkdir %HOME%
)

XCOPY phydm_commit_template.txt %HOME% /y

echo ======================
echo phydm commit template installed
pause