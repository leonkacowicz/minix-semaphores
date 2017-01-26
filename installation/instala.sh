#!/bin/sh

echo "Iniciando instalacao..."
echo "Copiando arquivos do Process Manager..."
cp semaf.c /usr/src/servers/pm/ > /dev/null 
cp table.c /usr/src/servers/pm/ > /dev/null 
cp proto.h /usr/src/servers/pm/ > /dev/null 
cp .depend /usr/src/servers/pm/ > /dev/null 
cp Makefile /usr/src/servers/pm/ > /dev/null 
echo "Concluido."

echo "Copiando biblioteca..."
cp Makefile_lib /usr/src/lib/posix/Makefile > /dev/null 
cp _sem_create.c /usr/src/lib/posix/ > /dev/null 
cp _sem_destroy.c /usr/src/lib/posix/ > /dev/null 
cp _sem_p.c /usr/src/lib/posix/ > /dev/null 
cp _sem_v.c /usr/src/lib/posix/ > /dev/null 

cp semaforo.h /usr/src/include/ > /dev/null
echo "Concluido."

echo "Compilando o Process Manager.."
cd /usr/src/servers/
make image > log_make_image.log
make install > log_make_install.log
echo "Conluido."

echo "Compilando o bibliotecas.."
cd /usr/src/
make libraries > log_make_libraries.log
echo "Conluido."

echo "Compilando o imagem de boot.."
cd /usr/src/tools/
make hdboot > log_make_hdboot.log
make install > log_make_install.log

echo "Conluido."
echo "Instalacao terminada."


