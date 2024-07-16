#include "mainEntradaSalida.h"

int main(int argc, char* argv[]) 
{
    decir_hola("IO");
   
    // Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    
    config_io = armar_config(log_io, argv[1]); // En argv[1] esta el nombre de config pasado por parametro

    log_info(log_io, "El nombre de la config es: %s", config_io->nombre);

    // Interfaz* configuracion_fs;

    //make start CONFIG_FILE=nombre_archivo.config (para arrancar) 

    // // Inicializar la configuración (ejemplo)
    // // configuracion_fs->archivo->block_size = 64;
    // // configuracion_fs->archivo->block_count = 1024;
    // //strcpy(configuracion_fs->archivo->path_base_dialfs, "/home/utnso/dialfs");

    // manejar_creacion_archivo("bloques.dat", configuracion_fs);
    // manejar_creacion_archivo("bitmap.dat", configuracion_fs);
    // manejar_creacion_archivo("notas.txt", configuracion_fs);

    // Las conexiones de sockets acá

    log_info(log_io, "Estoy antes de la conexion a kernel");
    conexion_io_kernel = conexion_a_kernel();

    pthread_t hilo_escuchar_memoria;

    if(strcmp(config_io->tipo_interfaz, "GENERICA") !=0 ) // Si no es la generica se conecta con memoria
    {
        log_info(log_io, "Estoy antes de la conexión a memoria");
        conexion_io_memoria = conexion_a_memoria(); 
        log_info(log_io, "Estoy después de la conexión a memoria");
        hilo_escuchar_memoria = escuchar_memoria();
    }

    // Conexion IO --> Kernel ---------------------------------------------------------------------------------------------------------
    
    pthread_t hilo_escuchar_kernel = escuchar_kernel();
    if(strcmp(config_io->tipo_interfaz, "GENERICA")!=0)
    {
        pthread_detach(hilo_escuchar_memoria);
    }
    
    pthread_join(hilo_escuchar_kernel, NULL);

    // terminar_programa(log_io, config_io);

    return 0;
}

void terminar_programa(t_log* log_io, t_config* config_io)
{
    if (log_io != NULL) 
    {
		log_destroy(log_io);
    }
    if (config_io != NULL) 
    {
		config_destroy(config_io);
    }
}