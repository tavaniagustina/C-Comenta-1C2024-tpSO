#ifndef CLIENTE_H_
#define CLIENTE_H_

#include "io_gestor.h"
#include "conexion.h"
#include "interfazGENERICA.h"
#include "interfazSTDIN.h"
#include "interfazSTDOUT.h"
#include "interfazDIALFS.h"
// #include "DIALFS.h"

// Declaracion de funciones ----------------------------------------------------------------------------------------------------------

int conexion_a_memoria();
int conexion_a_kernel();

op_code obtener_tipo_interfaz(char* char_interfaz);
void avisar_fin_io_a_kernel();

#endif