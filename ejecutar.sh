# Comprobar si se proporcionó el argumento
if [ -z "$1" ]
then
  echo "Error: Debes proporcionar el número de veces que se debe enviar la señal como argumento."
  exit 1
fi

# Compilar practicafinal.c
gcc -lpthread practicafinal.c -o final

# Ejecutar practicafinal.c en segundo plano y guardar su PID
./final &
PID=$!

# Esperar un poco
sleep 1

# Iniciar un bucle para enviar la señal N veces, donde N es el primer argumento del script
i=0
while [ $i -lt $1 ]
do
  # Enviar una señal al proceso
  kill -10 $PID
  # Esperar un poco antes de enviar la próxima señal
  sleep 0.5
  i=$(( $i + 1 ))
done

# Esperar a que el proceso termine
wait $PID