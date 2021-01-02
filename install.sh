#!/bin/bash

spacing=" ------------------------------------------------------------------ "
lang=$(cat $HOME/BorlandC-Linux/config/lang)
sleeptime=0.4
logo=$HOME/BorlandC-Linux/config/logo.sh

clear

sleep $sleeptime

bash $logo

sleep $sleeptime

echo "$spacing"
echo ""

sleep $sleeptime

if [[ $lang = "" ]]

	then

		echo " Select language: "
		echo ""
		echo " 0 | English "
		echo ""
		echo " 1 | Russian "
		echo ""

		sleep $sleeptime

		echo "$spacing "
		echo ""

		sleep $sleeptime

 		read -p " What language do you speak? > " choose

		case "$choose" in

			"0" )
			echo en > $HOME/BorlandC-Linux/config/lang; clear; bash $HOME/BorlandC-Linux/install.sh
			;;

			"1" )
			echo ru > $HOME/BorlandC-Linux/config/lang; clear; bash $HOME/BorlandC-Linux/install.sh
			;;

		esac


	else

		while [[ $lang = "en" ]]

			do

				echo " Using language: English "
				echo ""

				sleep $sleeptime

				echo " Preparing to install Borland C++ on Linux..."
				echo ""
				echo " All program files are in the cloned directory (including your future programs)"
				echo ""
				echo " More detailed information you can find on https://github.com/ddan9/BorlandC-Linux"
				echo "" 
				echo " If given a choice during installation, select and enter: Y"
				echo ""

				sleep 10

				echo "$spacing"
				echo ""

				apt install -y sudo
						
				echo ""

				sudo apt update
						
				echo ""

				sudo apt upgrade -y
						
				echo ""

				sudo apt install -y dosbox
						
				echo ""
						
				sudo apt dist-upgrade -y
						
				echo ""

				ln -s $HOME/BorlandC-Linux/start.sh $HOME/Borland.sh
						
				sleep 10

				echo ""
				echo "$spacing"
				echo ""

				sleep 2

				clear

				exit

			done

		while [[ $lang = "ru" ]]

			do

				echo " Используемый язык: Русский "
				echo ""

				sleep $sleeptime

				echo " Подготовка к установке Borland C++ на Линукс..."
				echo ""
				echo " Прошу вас заметить, что сама устанавливаемая программа по некоторым причинам не будет переведена на русский язык"
				echo ""
				echo " Все файлы программы лежат в клонированной директории (в том числе и ваших будущих программ)"
				echo ""
				echo " Более подробную информацию вы можете найти на https://github.com/ddan9/BorlandC-Linux"
				echo ""
				echo " Если во время установки вам будут давать выбор, выбирайте и вводите: Y"
				echo ""

				sleep 10

				echo "$spacing"
				echo ""

				apt install -y sudo
						
				echo ""

				sudo apt update
						
				echo ""

				sudo apt upgrade -y
						
				echo ""

				sudo apt install -y dosbox
						
				echo ""
						
				sudo apt dist-upgrade -y
						
				echo ""

				ln -s $HOME/BorlandC-Linux/start.sh $HOME/Borland.sh
						
				sleep 10

				echo ""
				echo "$spacing"
				echo ""

				sleep 2

				clear

				exit

			done

fi
