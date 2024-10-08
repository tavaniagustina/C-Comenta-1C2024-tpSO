#include "interfazDIALFS.h"

// IO_FS_CREATE -> Para crear un archivo
void manejar_creacion_archivo(char* nombre_archivo, int pid) 
{

    //Log obligatorio
    log_info(log_io, "PID: <%d> - Crear Archivo: <%s>", pid, nombre_archivo);

    //hago un usleep para lo q tarda
    usleep(config_io->tiempo_unidad_trabajo * 1000);

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);

    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/bitmap.dat");

    FILE* bitmap_file = fopen(path_base, "rb"); // Abro el "bitmap.dat"

    // char *bitmap_buffer = (char*)malloc(bitarray_size);

    // fread(bitmap_buffer, 1, bitarray_size, bitmap_file); // Acá almaceno el contenido del bitmap_file dentro del bitmap_buffer

    // fclose(bitmap_file); // Porque ya tengo el contenido en el buffer

    char* bitmap_buffer = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, MSB_FIRST);
    
    int index_primer_bloque_libre = bitarray_find_first_clear_bit(bitmap);

    if(index_primer_bloque_libre == -1)
    {
        log_error(log_io, "No hay bloques libres para la creacion de un nuevo archivo");
    }
    
    bitarray_set_bit(bitmap, index_primer_bloque_libre);

    bitmap_file = fopen(path_base, "wb");

    fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);

    fclose(bitmap_file);


    //----------------------------------------------------------------
    //sacar este log

    // FILE *file = fopen(path_base, "rb");


    // unsigned char byte;
    // while (fread(&byte, sizeof(unsigned char), 1, file) == 1) {
    //     for (int i = 7; i >= 0; i--) {
    //         printf("%d", (byte >> i) & 1);
    //     }
    //     printf(" "); // Para separar los bytes
    // }

    // fclose(file);

    // free(path_base);

    //----------------------------------------------------------------

    bitarray_destroy(bitmap);
    free(bitmap_buffer);
    
    // Armamos la metadata del archivo -------------------------------------
    //hay q sacarle al archivo el .txt, ahora se llama archivo1.txt

    char** parte = string_split(nombre_archivo, ".");
    
    nombre_archivo = parte[0];
    
    string_append(&nombre_archivo,".config");

    path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);

    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/");
    string_append(&path_base, nombre_archivo);

    char *config_file_path = path_base;

    // Crear la configuración desde el archivo
    //Tenemos q armar el archivo primero

    FILE* archivo_config = fopen(config_file_path, "w");
    
    fclose(archivo_config);
    
    t_config *config = config_create(config_file_path);
    
    if (config == NULL) 
    {
        fprintf(stderr, "Error: No se pudo cargar el archivo de configuración: %s\n", config_file_path);
        return;
    }

    char* bloque_inicial = pasar_a_string(index_primer_bloque_libre);
    
    config_set_value(config, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config, "TAMANIO_ARCHIVO", "0");

    // Guardar los cambios en el archivo
    config_save(config);
     

    // Liberar memoria utilizada por la configuración
    config_destroy(config);
    
    free(path_base);

    avisar_fin_io_a_kernel();
    
    return;
}

// IO_FS_DELETE -> Para eliminar archivo
void manejar_eliminacion_archivo(char* nombre_archivo, int pid) 
{
    //log obligatorio
    log_info(log_io, "PID: <%d> - Eliminar Archivo: <%s>", pid, nombre_archivo);

    usleep(config_io->tiempo_unidad_trabajo * 1000);

    char** parte = string_split(nombre_archivo, ".");

    nombre_archivo = parte[0];

    string_append(&nombre_archivo,".config");

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);

    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/");
    string_append(&path_base, nombre_archivo);
    
    char *config_file_path = path_base;

    t_config* config_aux = config_create(config_file_path);
    
    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
    int tamanio_en_bytes = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");


    config_destroy(config_aux);

    remove(config_file_path); // Elimino el archivo de metadata
    
    // Creamos un bitmap a partir del archivo y lo editamos

    int bloques_ocupados = calcular_bloques_que_ocupa(tamanio_en_bytes);

    char *bitmap_buffer = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, MSB_FIRST);

    for(bloque_inicial; bloque_inicial< bloques_ocupados; bloque_inicial ++) 
    {
        bitarray_clean_bit(bitmap, bloque_inicial); // pongo en cero los bloques ocupados
    }
    
    //tengo q actualizar el buffer
    
    escribir_archivo_con_bitmap(bitmap_buffer); // Actualizamos el archivo de bitmap

    free(bitmap_buffer);
    
    bitarray_destroy(bitmap);

    // no borramos los bloques del bloques.dat porque al marcar como libres los bloques en el bitmap se va a sobreescribir en esos bloques y listo

    //saco el archivo de la cola de archivos en fs

    char* nombre_aux;

    // for(int j = 0; j < queue_size(cola_archivos_en_fs); j++)
    // {
    //     nombre_aux = queue_pop(cola_archivos_en_fs);
    //     if(strcmp(nombre_aux , nombre_archivo) == 0)
    //     {
    //         j = queue_size(cola_archivos_en_fs) + 1;
    //     }else{
    //         queue_push(cola_archivos_en_fs, nombre_aux);
            
    //     }
    // }

    ////

    free(path_base);
    avisar_fin_io_a_kernel();

    return;
}

// IO_TRUNCATE
void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, int pid)
{
    //log obligatorio
    log_info(log_io, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%d>", pid, nombre_archivo, nuevo_tamanio);

    usleep(config_io->tiempo_unidad_trabajo * 1000);

//----------------------SACAR ESTE LOG-----------------

    // char* path_base_log = malloc(strlen(config_io->path_base_dialfs)+ 25);
    // strcpy(path_base_log, config_io->path_base_dialfs);
    // string_append(&path_base_log, "/bitmap.dat");
    
    // FILE *file = fopen(path_base_log, "rb");

    // unsigned char byte;
    // while (fread(&byte, sizeof(unsigned char), 1, file) == 1) {
    //     for (int i = 7; i >= 0; i--) {
    //         printf("%d", (byte >> i) & 1);
    //     }
    //     printf(" "); // Para separar los bytes
    // }

    // fclose(file);

    // free(path_base_log);

//----------------------SACAR ESTE LOG-----------------

    char** parte = string_split(nombre_archivo, ".");
    
    nombre_archivo = parte[0];
    
    string_append(&nombre_archivo,".config");

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/");
    string_append(&path_base, nombre_archivo);

    char *config_file_path = path_base;

    FILE* config_archivo = fopen(config_file_path, "r+");

    t_config* config_aux = config_create(config_file_path);

    t_metadata* metadata = malloc(sizeof(t_metadata));

    char* buffer_bitmap = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, MSB_FIRST);

    char* buffer;

    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
    int tamanio_original = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

    fclose(config_archivo);

    if(tamanio_original < nuevo_tamanio) // Vamos a tener que bsucar que haya bloques libres para agrandar el tamanio
    {
        int bytes_a_agregar = nuevo_tamanio - tamanio_original;

        int tamanio_ultimo_bloque = tamanio_original % config_io->block_size;

        int tamanio_disponible_ult_bloque = config_io->block_size - tamanio_ultimo_bloque;

        int bytes_a_agregar_en_bloques_enteros = bytes_a_agregar - tamanio_disponible_ult_bloque;
        
        if(tamanio_disponible_ult_bloque < bytes_a_agregar) // Si puedo meter todo lo que tengo en algun bloque ocupado lo hago ahi directamente
        {


            if(tamanio_disponible_ult_bloque > 0) // si los bloques ocupados no estan ocupados en su totalidad
            {
                bytes_a_agregar = bytes_a_agregar - tamanio_disponible_ult_bloque;
            }
            
            
            int bloques_necesarios = calcular_bloques_que_ocupa(bytes_a_agregar);
            

            //verificamos que haya la cantidad de bloques libres en total q necesitamos

            int cantidad_bloques_libres = contar_bloques_libres(bitmap);
            

            if(cantidad_bloques_libres < bloques_necesarios)
            {
                log_error(log_io, "No se puede realizar la opeacion TRUNCATE, no se posee el espacio necesario");
                return;
            }

            int bloque_inicial_nuevo = buscar_bloques_contiguos_desde_cierto_bloque(bloque_inicial, bloques_necesarios, bitmap);


            if(bloque_inicial_nuevo == -1) // Devuelve -1, no hay libres desde ese bloque
            {
                bloques_necesarios = calcular_bloques_que_ocupa(nuevo_tamanio); // Calculo de nuevo, porque van a ser todos desde una nueva posicion


                bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);

                
                if(bloque_inicial_nuevo == -1){ // No hay bloques contiguos, necesito compactar si entra
                    

                    // Primero tenemos q sacar los bloques ocupados y ppner la info en un buffer
                    // dsp compacto y pongo lo q habia sacado actuizando el bitmap
                    
                    int bloques_que_ocupa_original = calcular_bloques_que_ocupa(tamanio_original);
                    
                    buffer = leer_bloques(bloque_inicial, bloques_que_ocupa_original); // Este buffer tiene lo que ya venía en el archivo

                    // ahora marco en el bitmap esos bloques como libres
                    
                    for(int i = bloque_inicial; i < bloques_que_ocupa_original; i++)
                    {
                        bitarray_clean_bit(bitmap, i);
                    }
                    
                    //cierro todo y actualizo hasta ahora para q agarre el actualizado en compactar
                    escribir_archivo_con_bitmap(buffer_bitmap);
                    free(buffer_bitmap);
                    bitarray_destroy(bitmap);

                    compactar(pid); // compactar actualiza el bloques.dat y el bitmap
                    

                    //obtengo el bitmap actualizado dsp de compactar

                    buffer_bitmap = obtener_bitmap();
                    

                    bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, MSB_FIRST);

                    bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);
                    
                    if(bloque_inicial_nuevo == -1)
                    {
                        log_warning(log_io, "Sigue sin haber espacio contiguo");
                    }

                    int bloques_que_ocupa_nuevo = calcular_bloques_que_ocupa(nuevo_tamanio);

                    for(int i = bloque_inicial_nuevo; i < bloque_inicial_nuevo + bloques_que_ocupa_nuevo; i++)
                    {
                        
                        bitarray_set_bit(bitmap, i);
                    }

                    escribir_archivo_con_bitmap(buffer_bitmap);
                    free(buffer_bitmap);
                    agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

                    // tenemos q actualizar el config del archivo con el tamano y bloque inicial
                    metadata->nombre_archivo = nombre_archivo;
                    metadata->bloque_inicial = bloque_inicial_nuevo;
                    metadata->tamanio_archivo = nuevo_tamanio;

                    actualizar_metadata(metadata);

//----------------------------------------------------------------------------------------------
                    // path_base_log = malloc(strlen(config_io->path_base_dialfs)+ 25);

                    // strcpy(path_base_log, config_io->path_base_dialfs);

                    // string_append(&path_base_log, "/bitmap.dat");

                    // FILE *file_aux = fopen(path_base_log, "rb");

                    // unsigned char byte_aux;

                    // while (fread(&byte_aux, sizeof(unsigned char), 1, file_aux) == 1) {
                    //     for (int i = 7; i >= 0; i--) {
                            
                    //         printf("%d", (byte_aux >> i) & 1);
                                            
                    //     }

                    //     printf(" "); // Para separar los bytes

                    // }

                    // fclose(file_aux);
                    // free(path_base_log);

            //----------------------------------------------------------------------------------------------
                    
                }
                else
                {   
                    // hay bloques contiguos, tengo q ponerlo en esos bloques


                    int bloques_del_tamanio_original = calcular_bloques_que_ocupa(tamanio_original);
                    
                    if (bloques_del_tamanio_original == 0){
                        bloques_del_tamanio_original = 1;
                    }
                    
                    // Deberia dar 1 en el caso del FS_1

                    buffer = leer_bloques(bloque_inicial, bloques_del_tamanio_original);

                    

                    for(int i = bloque_inicial; i < bloque_inicial + bloques_del_tamanio_original; i++)
                    {
                        bitarray_clean_bit(bitmap, i);
                    }
                    
                    agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

                    // marco los nuevos bits como ocupados
                    for(int i = bloque_inicial_nuevo; i < bloque_inicial_nuevo + bloques_necesarios; i++)
                    {
                        bitarray_set_bit(bitmap, i);
                    }
                    
                    // guardo la metadata actualidad
                    metadata->tamanio_archivo = nuevo_tamanio;
                    metadata->nombre_archivo = nombre_archivo;
                    metadata->bloque_inicial = bloque_inicial_nuevo;

                    //log_info(log_io, "El bloque de inicio que estoy por mandar a metadata es: %d", bloque_inicial_nuevo);

                    // actualizo los dos archivos
                    //tengo q mandarle el del bitmap_buffer no el bitmap
                    escribir_archivo_con_bitmap(buffer_bitmap);
                    free(buffer_bitmap);
                    actualizar_metadata(metadata);
                }
            }else{

                // tenemos en bloque_inicial el bloque inicial
                metadata->tamanio_archivo = nuevo_tamanio;
                metadata->nombre_archivo = nombre_archivo;
                metadata->bloque_inicial = bloque_inicial;


                //bloque_inicial + 1 porq el inicial ya esta seteado
                //bloques_necesarios + bloque_inicial para q sean los necesarios desde el inicial

               

                for(int i = bloque_inicial + 1; i < bloques_necesarios + bloque_inicial + 1; i++)
                {
                    bitarray_set_bit(bitmap,i);
                }

                //le mando el buffer
                escribir_archivo_con_bitmap(buffer_bitmap);
                free(buffer_bitmap);
                actualizar_metadata(metadata);
            }
        }else{
            
            //actualizo el metadata solamente, el bitmap no porq no le agregue bloques
            metadata->tamanio_archivo = nuevo_tamanio;
            metadata->nombre_archivo = nombre_archivo;
            metadata->bloque_inicial = bloque_inicial;

            actualizar_metadata(metadata);
        }

        

    }else if(tamanio_original > nuevo_tamanio) // solo tengo que truncar el archivo, marco el bitmap como libre
    { 
        int bloques_a_liberar = calcular_bloques_que_ocupa(tamanio_original) - calcular_bloques_que_ocupa(nuevo_tamanio);
        
        for(int i = bloque_inicial; i < bloque_inicial + bloques_a_liberar ; i++ )
        {
            bitarray_clean_bit(bitmap, i);
        }

        metadata->tamanio_archivo = nuevo_tamanio;
        metadata->nombre_archivo = nombre_archivo;
        metadata->bloque_inicial = bloque_inicial;

        escribir_archivo_con_bitmap(buffer_bitmap);
        free(buffer_bitmap);
        actualizar_metadata(metadata);
    }

    
    config_destroy(config_aux);
    //free(buffer_bitmap);
    bitarray_destroy(bitmap);

    //----------------------SACAR-----------------------
    // path_base_log = malloc(strlen(config_io->path_base_dialfs)+ 25);
    // strcpy(path_base_log, config_io->path_base_dialfs);    
    // string_append(&path_base_log, "/bitmap.dat");

    // FILE *file_2 = fopen(path_base_log, "rb");


    // unsigned char byte_2;
    // while (fread(&byte_2, sizeof(unsigned char), 1, file_2) == 1) {
    //     for (int i = 7; i >= 0; i--) {
    //         printf("%d", (byte_2 >> i) & 1);
    //     }
    //     printf(" "); // Para separar los bytes
    // }

    // fclose(file_2);
    
    // free(path_base_log);

    //---------------------------------------------

    
    avisar_fin_io_a_kernel();

    free(path_base);

    return;
}   

// IO_WRITE
void manejar_escritura_archivo(char* nombre_archivo, t_list* direccion_fisica, int tamanio, int puntero_archivo, int pid)
{
    //log obligatorio
    log_info(log_io, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero_archivo);
    
    usleep(config_io->tiempo_unidad_trabajo * 1000);

    // Primero Memoria tiene que leer la direccion para obtener el valor (como en el STDOUT WRITE, recien despues de eso puedo escribir el archivo)

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    agregar_lista_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_list));

    enviar_paquete(paquete, conexion_io_memoria);

    eliminar_paquete(paquete);

    sem_wait(&sem_ok_lectura_memoria);

    // Ahora ya tengo los datos que quiero escribir en el archivo en "valor_a_mostrar" (es global)


    //escribimos en el archivo lo q traje de memoria
    
    char** parte = string_split(nombre_archivo, ".");
    
    nombre_archivo = parte[0];

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&nombre_archivo,".config");

    string_append(&path_base, "/");

    string_append(&path_base, nombre_archivo);

    t_config* config_aux = config_create(path_base);

    int tamanio_archivo = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");
    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");

    config_destroy(config_aux);
    
    
    free(path_base);
    path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
   strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/bloques.dat");

    FILE* archivo_bloques = fopen(path_base, "r+");

    int byte_inicio_bloque = bloque_inicial * config_io->block_size;

    //Pongo el puntero al inicio de los bloques del archivo
    fseek(archivo_bloques, byte_inicio_bloque , SEEK_SET);

    //Pongo el puntero en la posicion a partir de la cual quiero leer
    fseek(archivo_bloques, puntero_archivo, SEEK_CUR);

    fwrite(valor_a_mostrar, sizeof(char), tamanio, archivo_bloques);

    fclose(archivo_bloques);

    free(path_base);
    free(valor_a_mostrar);

    free(parte[0]);
    free(parte[1]);
    free(parte);

    avisar_fin_io_a_kernel();

    return;
}

// IO_READ 
void manejar_lectura_archivo(char* nombre_archivo, t_list* direccion_fisica, int tamanio, int puntero_archivo, int pid)
{

    // IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo): Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde el archivo a partir del valor del Registro Puntero Archivo la cantidad de bytes indicada por Registro Tamaño y se escriban en la Memoria a partir de la dirección lógica indicada en el Registro Dirección.

    //log obligatorio
    log_info(log_io, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero_archivo);

    usleep(config_io->tiempo_unidad_trabajo * 1000);


    //cambio el nombre del archivo

    char** parte = string_split(nombre_archivo, ".");

    nombre_archivo = parte[0];
    
    string_append(&nombre_archivo,".config");

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);
    string_append(&path_base, "/");
    string_append(&path_base, nombre_archivo);

    //leemos del archivo 
        
    t_config* config_aux = config_create(path_base);

    int tamanio_archivo = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");
    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");

    config_destroy(config_aux);

    free(path_base);
    path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);
    
    string_append(&path_base, "/bloques.dat");

    FILE* archivo_bloques = fopen(path_base, "r");

    int byte_inicio_bloque = bloque_inicial * config_io->block_size;

    //Pongo el puntero al inicio de los bloques del archivo
    fseek(archivo_bloques, byte_inicio_bloque , SEEK_SET);
    //Pongo el puntero en la posicion a partir de la cual quiero leer
    fseek(archivo_bloques, puntero_archivo, SEEK_CUR);

    char* buffer_datos = malloc(sizeof(char)*(tamanio+1));

    fread(buffer_datos, sizeof(char), tamanio, archivo_bloques);


    fclose(archivo_bloques);

    free(path_base);

    // MANDAMOS A MEMORIA para q guarde

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_ESCRITURA_MEMORIA); // Queremos que memoria lo guarde

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    agregar_string_al_paquete_personalizado(paquete, buffer_datos);
    agregar_lista_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_direccion_fisica));
    
    enviar_paquete(paquete, conexion_io_memoria);

    eliminar_paquete(paquete);

    free(buffer_datos);

    t_direccion_fisica* dir;

    for(int i = 0; i < list_size(direccion_fisica); i++)
    {
        dir = list_get(direccion_fisica, i);
        free(dir);
    }

    list_destroy(direccion_fisica);

    sem_wait(&sem_ok_escritura_memoria);

    free(parte[0]);
    free(parte[1]);
    free(parte);

    avisar_fin_io_a_kernel();

    return;
}

void crear_archivos_gestion_fs()
{
    // Creo la carpeta en caso de que no existe
    struct stat st = {0};
    
    // Verificar si la carpeta ya existe
    if (stat(config_io->path_base_dialfs, &st) == -1) {
        // Crear la carpeta
        if (mkdir(config_io->path_base_dialfs, 0777) == -1) {
            perror("Error al crear la carpeta");
            exit(EXIT_FAILURE);
        } else {
            printf("Carpeta creada exitosamente\n");
        }
    } else {
        printf("La carpeta ya existe\n");
    }
    // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bloques.dat

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);

    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/bloques.dat");


    FILE* bloques_dat = fopen(path_base, "r");


    if(bloques_dat == NULL){
        // El archivo no existe, se debe crear e inicializar

        FILE* bloques_dat = fopen(path_base, "wb");

       
        
        if (bloques_dat == NULL) 
        {
            log_error(log_io, "Error al abrir el archivo");
            return;
        }

        int tamanio = config_io->block_size * config_io->block_count;


        // Mueve el puntero al final del archivo para definir su tamaño
        if (fseek(bloques_dat, tamanio - 1, SEEK_SET) != 0) 
        {
            log_error(log_io,"Error al establecer el tamaño del archivo");
            fclose(bloques_dat);
            return;
        }

        // Escribe un byte en la última posición para establecer el tamaño
        fputc('\0', bloques_dat);


        fclose(bloques_dat);


    }else{

        fclose(bloques_dat);
    }


    free(path_base);

    // CREO EL ARCHIVO CON EL BITMAP --------------------------------------------------------
    path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/bitmap.dat");
    
    
    FILE* bitmap_file = fopen(path_base, "r");

    if(bitmap_file == NULL){

        // bitmap.dat no existe, tengo que crearlo e inicializarlo

        FILE *bitmap_file = fopen(path_base, "wb");  // Abre el archivo en modo lectura y escritura binaria
        
        if (bitmap_file == NULL) 
        {
            log_error(log_io, "Error al abrir bitmap.dat");
            return;
        }
        
       
        size_t bitmap_size_bytes = (config_io->block_count + 7) / 8;  // 1 bit por bloque
        
        char* bitmap_buffer = (char*)malloc(bitmap_size_bytes);

        //log_info(log_io,"arme el bitmap buffer");

        if (bitmap_buffer == NULL) 
        {
            perror("Error al reservar memoria para el bitmap");
            fclose(bitmap_file);
            return;
        }
        
        memset(bitmap_buffer, 0, bitmap_size_bytes);
        // Limpio el bitmap -> lo seteo en 0
        t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, MSB_FIRST);
        

        fseek(bitmap_file, 0, SEEK_SET);  // Mueve el puntero al inicio del archivo
        fwrite(bitmap_buffer, sizeof(char), bitmap_size_bytes, bitmap_file);

    
        if (bitmap == NULL) 
        {
            fprintf(stderr, "Error al crear el bitmap\n");
            free(bitmap_buffer);
            fclose(bitmap_file);
            return;
        }

        free(bitmap_buffer);
        fclose(bitmap_file);
        

        bitarray_destroy(bitmap);
        
        
        // bitarray_set_bit(bitmap, 0);  // Por ejemplo, establece el primer bloque como ocupado
        // bitarray_set_bit(bitmap, 1);  // Y el segundo bloque como ocupado
        // bitarray_clean_bit(bitmap, 0) // Pone el bit número 0 libre
    }else{

        fclose(bitmap_file);
    }
    free(path_base);
    return;
}

char* pasar_a_string(int valor)
{
    char* buffer = (char*)malloc(20 * sizeof(char));
    if (buffer != NULL)
    {
        snprintf(buffer, 20, "%d", valor);
    } else {
        log_error(log_io, "Error.");
    }
    return buffer;
}


int calcular_bloques_que_ocupa(int bytes)
{
    int bloques_que_ocupa = (bytes + config_io->block_size - 1) / config_io->block_size;
    
    return bloques_que_ocupa;
}

char* obtener_bitmap()
{
    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/bitmap.dat");
    
    FILE* bitmap_file = fopen(path_base, "rb"); 

    //bitarray_size = (config_io->block_count + 7) / 8;

    char *bitmap_buffer = (char *)malloc(bitarray_size);
    fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);
    free(path_base);
    return bitmap_buffer;
}

void escribir_archivo_con_bitmap(char* bitmap_buffer)
{
//-------------------------SACAR----------------

    // char* path_base_log = malloc(strlen(config_io->path_base_dialfs)+ 25);
    // strcpy(path_base_log, config_io->path_base_dialfs);
    // string_append(&path_base_log, "/bitmap.dat");

    // FILE *file_2 = fopen(path_base_log, "rb");


    // unsigned char byte_2;
    // while (fread(&byte_2, sizeof(unsigned char), 1, file_2) == 1) {
    //     for (int i = 7; i >= 0; i--) {
    //         printf("%d", (byte_2 >> i) & 1);
    //     }
    //     printf(" "); // Para separar los bytes
    // }

    // fclose(file_2);
    
//-----------------------------------------

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);
    string_append(&path_base, "/bitmap.dat");

    FILE* bitmap_file = fopen(path_base, "wb"); 
    fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);
    free(path_base);

//--------------------SACAR---------------------

    // path_base_log = malloc(strlen(config_io->path_base_dialfs)+ 25);
    // strcpy(path_base_log, config_io->path_base_dialfs);
    
    // FILE *file_3 = fopen(path_base_log, "rb");


    // unsigned char byte_3;
    // while (fread(&byte_3, sizeof(unsigned char), 1, file_3) == 1) {
    //     for (int i = 7; i >= 0; i--) {
    //         printf("%d", (byte_3 >> i) & 1);
    //     }
    //     printf(" "); // Para separar los bytes
    // }
    // fclose(file_3);
    // free(path_base_log);

//-----------------------------------------


   
}

int buscar_bloques_contiguos_desde_cierto_bloque(int bloque_inicial, int bloques_necesarios, t_bitarray* bitmap) 
{
    int cont_bloques_libres_consecutivos = 0;

    for (int i = bloque_inicial+1; i < config_io->block_count; i++) 
    {
        if (!bitarray_test_bit(bitmap, i)) 
        { // Si el bloque i está libre (bit es 0)
            
            cont_bloques_libres_consecutivos++; // Incrementa el cont de bloques libres consecutivos

            if (cont_bloques_libres_consecutivos == bloques_necesarios) 
            {
                return bloque_inicial; // Retorna bloque_inicial si se encontraron suficientes bloques libres consecutivos
            }
        } else {
            return -1; // Reinicia el cont si se encuentra un bloque ocupado
        }
    }

    return -1; // Retorna -1 si no se encuentran suficientes bloques libres contiguos
}

int buscar_bloques_contiguos(int bloques_necesarios, t_bitarray* bitmap){
    
    int contador_bloques_contiguos = 0;
    int bloque_inicio = -1; // Inicializa bloque_inicio

    for (int i = 0; i < config_io->block_count; i++) 
    {
        if (!bitarray_test_bit(bitmap, i)) 
        { 
            // Bloque libre
            if (contador_bloques_contiguos == 0) 
            {
                bloque_inicio = i;
                

            }

            contador_bloques_contiguos++;


            if (contador_bloques_contiguos == bloques_necesarios)
            {
                
                return bloque_inicio;
            }
            
        } else {
            contador_bloques_contiguos = 0;
            bloque_inicio = -1;
        }
    }
    return -1; // No hay suficientes bloques contiguos disponibles 
}

int contar_bloques_libres(t_bitarray* bitmap)
{
    int bloques_libres = 0;

    for (int i = 0; i < config_io->block_count; i++)
    {
        if (!bitarray_test_bit(bitmap, i)) 
        { // Si el bit es 0, el bloque está libre,
            bloques_libres++;
        }
    }
    
    return bloques_libres;
}

char* leer_bloques(int bloque_inicial, int num_bloques) 
{
    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);
    
    string_append(&path_base, "/bloques.dat");
    FILE* archivo = fopen (path_base, "r+");
    
    // Calcular el offset al inicio del primer bloque
    int offset = bloque_inicial * config_io->block_size;

    // Mover el puntero al inicio del primer bloque
    fseek(archivo, offset, SEEK_SET);

    // Leer el contenido de los bloques
    char *buffer = malloc(num_bloques * config_io->block_size + 1);


    fread(buffer, config_io->block_size, num_bloques, archivo);

    fclose(archivo);
    
    free(path_base);


    return buffer;
}

char *agregar_al_final(char *buffer, char *informacion) 
{
    

    if (buffer == NULL) 
    {
        
        // Si el buffer es NULL, asigna memoria suficiente para la información
        buffer = malloc(strlen(informacion) + 1);  // +1 para el terminador nulo
        

        strcpy(buffer, informacion);
        free(informacion);
    } else {
        // Si el buffer ya contiene datos, realloca memoria para incluir la nueva información
        size_t tam_buffer = strlen(buffer);
        size_t tam_informacion = strlen(informacion);
        buffer = realloc(buffer, tam_buffer + tam_informacion + 1);  // +1 para el terminador nulo
        if (buffer == NULL) 
        {
            perror("Error al realocar memoria");
            return NULL;
        }
        strcat(buffer, informacion);
        free(informacion);
        
    }
    return buffer;
}

// Función para compactar bloques
void compactar(int pid) 
{ 
    //log obligatorio
    log_info(log_io, "PID: <%d> - Inicio Compactacion.", pid);

    char* buffer_bitmap = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, MSB_FIRST);
    
    //--------------------------------__SACAR-----------------------------

    // char* path_base_log = malloc(strlen(config_io->path_base_dialfs)+ 25);
    // strcpy(path_base_log, config_io->path_base_dialfs);
    
    // string_append(&path_base_log, "/bitmap.dat");

    // FILE *file_aux = fopen(path_base_log, "rb");

    // unsigned char byte_aux;
    // while (fread(&byte_aux, sizeof(unsigned char), 1, file_aux) == 1) {
    //     for (int i = 7; i >= 0; i--) {
    //         printf("%d", (byte_aux >> i) & 1);
    //     }
    //     printf(" "); // Para separar los bytes
    // }

    // fclose(file_aux);   
    // free(path_base_log); 
    //------------------------------------------------------------------------
    
    int tamanio_maximo_de_bloques_dat = config_io->block_count * config_io->block_size;
    
    
    //char* buffer ;
    int contador_ocupados = 0;
    //No hago el malloc porq buscar archivo q inicia ya devuelve uno con malloc hecho
    t_metadata* metadata; // = malloc(sizeof(t_metadata)); //metadata->nombre_archivo, ->BLOQUE_INICIAL, ->TAMANIO_ARCHIVO
    int bloques_ocupados;
    char* info_bloques;
    //i recorre cada bloque del bitmap i == un numero de bloque

    for (int i = 0; i < config_io->block_count; i++) 
    { 

        if(bitarray_test_bit(bitmap, i)) //si entro es porq esta ocupado
        {
            metadata = buscar_archivo_que_inicia_en_bloque(i);
            
            bloques_ocupados = calcular_bloques_que_ocupa(metadata->tamanio_archivo);

            info_bloques = leer_bloques(i , bloques_ocupados);
            
            metadata->bloque_inicial = contador_ocupados;
            
            agregar_info_en_cierto_bloque(metadata->bloque_inicial, bloques_ocupados, info_bloques);

            for(int h = 0; h < bloques_ocupados; h++) 
            {
                bitarray_set_bit(bitmap, contador_ocupados + h);
                bitarray_clean_bit(bitmap, i + h); // Limpia los bloques viejos
            }
            
            contador_ocupados += bloques_ocupados;

            actualizar_metadata(metadata);

            i += bloques_ocupados - 2; //le sacamos un -1

        }

        
    }
    //actualizo el archivo de bitmap
    escribir_archivo_con_bitmap(buffer_bitmap);
    
    bitarray_destroy(bitmap);
    free(buffer_bitmap);
    

    usleep(config_io->retraso_compactacion * 1000); //multiplico por mil porque esta dado en miliseg y necesito microseg
    //log obligatorio
    log_info(log_io, "PID: <%d> - Fin Compactacion.", pid);

    return ;
}

void actualizar_archivo_bloques(char* buffer)
{

    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);

    strcpy(path_base, config_io->path_base_dialfs);
    string_append(&path_base, "/bloques.dat");
    FILE * archivo_bloques = fopen(path_base, "rb+");
    fwrite(buffer, config_io->block_count * config_io->block_size, 1, archivo_bloques);
    fclose(archivo_bloques);
    free(buffer);
    free(path_base);
}

t_metadata* buscar_archivo_que_inicia_en_bloque(int nro_bloque)
{
    //HAAY QUE CAMBIAR TODA, AHORA NO BUSCAMOS EN LA COLA
    t_metadata* meta_aux = malloc(sizeof(t_metadata));
    meta_aux->nombre_archivo = malloc(sizeof(char)*25);
    t_config* config_aux;
    struct dirent *ent; //entrada del directorio
    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    // strcpy(path_base, config_io->path_base_dialfs);
    
    DIR* dir;
    if(dir = opendir(config_io->path_base_dialfs))
    {
        while ((ent = readdir(dir)) != NULL) {
            
            // meta_aux->nombre_archivo = ent->d_name;
            strcpy(meta_aux->nombre_archivo, ent->d_name);

            if(strcmp(meta_aux->nombre_archivo, "bloques.dat")!=0 && strcmp(meta_aux->nombre_archivo, "bitmap.dat")!=0 && strcmp(meta_aux->nombre_archivo, "..") != 0 && strcmp(meta_aux->nombre_archivo, ".") != 0)
            {
                strcpy(path_base, config_io->path_base_dialfs);
                string_append(&path_base, "/");
                string_append(&path_base, meta_aux->nombre_archivo);
                config_aux = config_create(path_base);
                meta_aux->bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
                meta_aux->tamanio_archivo = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

                if(meta_aux->bloque_inicial == nro_bloque)
                {
                    //encontre el q queria
                    config_destroy(config_aux);
                    break;
                    
                }else config_destroy(config_aux);
            }
            
        }
    }else log_error(log_io , "ERROR AL ABRIR LA CARPETA");

    closedir(dir);
    free(path_base);

    return meta_aux;
}

void actualizar_metadata(t_metadata* metadata)
{
    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);

    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/");
    string_append(&path_base, metadata->nombre_archivo);

    t_config* config_archivo = config_create(path_base);

    char* bloque_inicial = pasar_a_string(metadata->bloque_inicial);
    char* tamanio_bloque = pasar_a_string(metadata->tamanio_archivo);

    config_set_value(config_archivo, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config_archivo, "TAMANIO_ARCHIVO", tamanio_bloque);

    config_save(config_archivo);

    config_destroy(config_archivo);

    free(path_base);

    //fclose(archivo);

    free(metadata->nombre_archivo);
    free(metadata);
}

int agregar_info_en_cierto_bloque(int bloque_inicial_nuevo, int cant_bloques , char* buffer)
{
    char* path_base = malloc(strlen(config_io->path_base_dialfs)+ 25);
    strcpy(path_base, config_io->path_base_dialfs);

    string_append(&path_base, "/bloques.dat");
    
    FILE * archivo_bloques = fopen(path_base, "rb+");
    int offset = bloque_inicial_nuevo * config_io->block_size;
    fseek(archivo_bloques, offset, SEEK_SET);
    fwrite(buffer, config_io->block_size, cant_bloques, archivo_bloques);
    fclose(archivo_bloques);
    free(path_base);
    free(buffer);
    return 1;
}

int bitarray_find_first_clear_bit(t_bitarray* bitmap)
{
    for (int i = 0; i < config_io->block_count; i++)
    {
        

        if (!bitarray_test_bit(bitmap, i)) // Si está vacío el bitmap, entonces devuelvo el índice en el que me encuentro
        {
            return i;
        }
    }
    return -1;
}

int min(int a, int b) {
    return a < b ? a : b;
}