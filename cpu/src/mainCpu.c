#include <mainCpu.h>

int main(int argc, char* argv[]) 
{
    decir_hola("CPU");
    
	// Log y config de uso general ----------------------------------------------------------------------------------------------------

	log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
	config_cpu = armar_config(log_cpu);
	inicializar_tlb();
	
	// Semaforo para traducir
	sem_init(&sem_tengo_el_marco, 0, 0);
	sem_init(&sem_tengo_ok_resize, 0, 0);
	sem_init(&sem_hay_instruccion, 0, 0);


	flag_interrupcion = false;

	motivo_interrupcion = -1;


    // Conexion CPU --> Memoria -------------------------------------------------------------------------------------------------------

	socket_cliente_cpu = conexion_a_memoria();

	// Instrucciones con memoria
	
	pthread_t hilo_escuchar_memoria = escuchar_memoria();

	// Inicializo los servidores para kernel ----------------------------------
	
	server_para_kernel();
    interrupcion_para_kernel();

	// hilos para recibir a Kernel ---------------------------------------------------------------------------------------------------
	
	pthread_t hilo_escuchar_kernel = escuchar_kernel();
	
	pthread_t hilo_escuchar_kernel_interrupcion = escuchar_kernel_interrupcion();

	pthread_t hilo_interpretar_instruccion = crear_hilo_interpretar_instruccion();
	

	pthread_detach(hilo_escuchar_kernel);
	
	pthread_detach(hilo_escuchar_kernel_interrupcion);

	pthread_join(hilo_escuchar_memoria, NULL);
   
	terminar_programa();
	
	liberar_conexion(socket_cliente_kernel);
	return EXIT_SUCCESS;
}

void terminar_programa()
{
	sem_destroy(&sem_tengo_el_marco);
	sem_destroy(&sem_tengo_ok_resize);
	sem_destroy(&sem_hay_instruccion);
	
	if (log_cpu != NULL) 
    {
		log_destroy(log_cpu);
	}
	if (config_cpu != NULL) 
    {
		config_destroy(config_cpu);
	}
}