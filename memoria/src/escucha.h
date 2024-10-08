#ifndef ESCUCHA_H_
#define ESCUCHA_H_

#include "m_gestor.h"
#include "servidor.h"
#include "atenderKERNEL.h"
#include "atenderCPU-IO.h"

// ****************** DECLARACIÓN DE FUNCIONES ************************** 
// ******* DESARROLLO DE FUNCIONES DE ESCUCHA ACTIVA PARA LOS MODULOS *********
void atender_cpu();
void atender_kernel();
// void atender_io();


// ******* HILOS PARA ESCUCHA ACTIVA *********
void recibir_escuchar_cpu();
void recibir_escuchar_kernel();
void recibir_escuchar_io();

#endif