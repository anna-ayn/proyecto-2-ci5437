# !/bin/bash

# Uso: ./getResults.sh <archivo de resultados>

# Este script se encarga de obtener los resultados de la ejecución de los programas en el formato:
# <Nivel i>. <White/Black> moves: value=<Valor>, #expanded=<Nodos expandidos>, #generated=<Nodos generados>, seconds=<Tiempo de ejecución>, #generated/second=<Generados*segundo>
# Para cada nivel, se obtiene el resultado de los movimientos de las piezas blancas y negras. Y se muestran por consola en el formato:
# <Nivel i>\t <Valor>\t <Nodos expandidos>\t <Nodos generados>\t <Tiempo de ejecución>\t <Generados*segundo>

# Se verifica que se haya pasado un argumento
if [ "$#" -ne 1 ]; then
    echo "Uso: ./getResults.sh <archivo de resultados>"
    exit 1
fi

# Se verifica que el archivo de resultados exista
if [ ! -f $1 ]; then
    echo "El archivo de resultados no existe"
    exit 1
fi

# Se crea un archivo temporal para guardar los resultados
TEMP="temp.txt"

# Se obtiene el nombre del archivo de resultados
resultFile=$1

# Se obtiene la cantidad de líneas del archivo y le restamos 3 (Encabezado) para obtener la cantidad de niveles
levels=$((`wc -l $resultFile | awk '{print $1}'` - 3))

# Se imprime el encabezado
echo -e "Nivel\t Valor\t Nodos expandidos\t Nodos generados\t Tiempo de ejecución\t Generados*segundo"

# Se toman las lineas del archivo que contienen los resultados y se guardan en un archivo temporal
tail -n $levels $resultFile > $TEMP

# Se recorren las líneas de los resultados (Archivo temporal)
while IFS= read -r line
do
    # Se obtiene el nivel sin el punto
    level=`echo $line | awk '{print $1}' | tr -d '.'`
    # Se obtiene el valor
    value=`echo $line | awk '{print $4}' | tr -d ',' | tr -d 'value='`
    # Se obtienen los nodos expandidos
    expanded=`echo $line | awk '{print $5}' | tr -d ',' | tr -d '#expanded='`
    # Se obtienen los nodos generados
    generated=`echo $line | awk '{print $6}' | tr -d ',' | tr -d '#generated='`
    # Se obtiene el tiempo de ejecución
    timeL=`echo $line | awk '{print $7}' | tr -d ',' | tr -d 'seconds=' | tr '.' ','`
    # Se obtiene la cantidad de nodos generados por segundo (Puede ser -Inf, nan o un número real). Tambien remplaza el . por ,
    generatedPerSecond=`echo $line | cut -d'=' -f6 | tr '.' ','` 
    # Se imprime el resultado en forma de tabla
    echo -e "$level\t $value\t $expanded\t $generated\t $timeL\t $generatedPerSecond"
done < $TEMP

# Se elimina el archivo temporal
rm $TEMP