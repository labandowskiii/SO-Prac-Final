#!/bin/bash


function mostrar(){
        cat practicafinal.c
}


function compilar(){
        gcc -pthread practicafinal.c -o final
}

function ejecutar(){
        echo "introduzca el numero de clientes a crear"
        read clientes
        if [ $clientes -lt 1 ]
        then
                echo -e "El numero de clientes debe ser mayor a 0\n"
                ejecutar
        elif test -f final
        then
                if test -x final
                then
					./ejecutar.sh $clientes
                else
                        echo -e "El archivo no tiene permisos de ejecucion\n"

                fi
        else
                echo -e "Primero debe compilar el archivo\n"
        fi
}

while true
do
echo -e "Seleccione una opcion:\n"
echo -e "1.-Mostrar el codigo\n"
echo -e "2.-Compilar el codigo\n"
echo -e "3.-Ejecutar el codigo\n"
echo -e "4.-Salir del programa\n"


read input

        case $input in

                1)mostrar
                 ;;
                2)compilar
                 ;;
                3)ejecutar
                 ;;
                4)break
                 ;;
                *) echo -e "Escoja una de las 4 opciones"

        esac
done


#final

