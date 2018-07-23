#ifndef REGION_H
#define REGION_H

#include <pthread.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <semaphore.h>


/*
    Estructura para esta versión de la región que facilita el control de las variables a
    compartir y los avisos para cuando ya se puede acceder a la región.
*/
struct sharedValues
{
    public:
    std::string name;    
    sem_t sem;
    bool waitFor;
    sharedValues()      
        :sem()
        ,waitFor(false)
    {

    }
};

/*
    @Summary: Macro DESTROY_REGION para destruir todos los recursos creados al usar las regiones
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
 */

#define DESTROY_REGION( regionName ) \
    for ( size_t index = 0; index < maxR##_##regionName; ++ index )\
        {\
            pthread_mutex_destroy( &maxSpacesForRegion##_##regionName[ index ] );\
        };\
    for ( size_t index2 = 0; index2 < maxR##_##regionName; ++ index2 )\
        {\
            sem_destroy( &semsPerRegion##_##regionName[ index2 ].sem );\
        };\


/*
    @Summary: Macro DESTROY_REGION para destruir todos los recursos creados al usar las regiones
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param maxRegionCalls: Numero máximo de espacios de región para la región actualmente a definir
*/

#define INIT_REGION( regionName, maxRegionCalls )\
    pthread_mutex_t maxSpacesForRegion##_##regionName[ maxRegionCalls ];\
    sharedValues semsPerRegion##_##regionName[ maxRegionCalls ];\
    void initialize##_##regionName##_##aaabbbccc()\
    {\
        for (int x = 0; x < maxRegionCalls; ++x ) \
        {\
            sem_init( &semsPerRegion##_##regionName[x].sem,0,0 );\
            pthread_mutex_init(&maxSpacesForRegion##_##regionName[ x ],NULL);\
        }\
    } \
    static size_t maxR##_##regionName = maxRegionCalls; \
    static bool  controlInit##_##regionName = false; \

/*
    @Summary: Macro SHARED para "compartir" una variable global en una región específica.
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param variableNames(...): Nombre del identificador de la variable para declarar como compartida en la región
*/
#define SHARED( regionName,...) \
    if ( controlInit##_##regionName == false ) \
    { \
        initialize##_##regionName##_##aaabbbccc();\
        controlInit##_##regionName = true;\
    } \

/*
    @Summary: Macro REGION que declara un espacio a usar después de la incialización de la región como tal.
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param spaceNumber: Número del espacio de la región a utilizar.
*/
#define REGION( regionName,spaceNumber )\
    pthread_mutex_lock( &maxSpacesForRegion##_##regionName[ spaceNumber ] );\

/*
    @Summary: Macro WHEN que detiene el acceso al región crítica.
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param spaceNumber: Número del espacio de la región a utilizar.
    @Param condition: Método o condición booleana para entrar a ejecutarse en la región.
*/
#define WHEN( regionName, spaceNumber, condition )\
    semsPerRegion##_##regionName[ spaceNumber ].waitFor = true;\
    while ( (condition) == false )\
    {\
           sem_wait( &semsPerRegion##_##regionName[ spaceNumber ].sem );\
    };\

/*
    @Summary: Macro DO para realizar lo que se desee cuando se entre a la región.
    @Param regionName: Nombre de la region, debe ser texto. Preferiblemente una definición de código
    @Param spaceNumber: Número del espacio de la región a utilizar.
    @Ṕaram Code: Literalmente el código a ejecutar en la región.
*/
#define DO(regionName,spaceNumber,Code)\
    Code\
    for (size_t x = 0; x < maxR##_##regionName; ++ x )\
    {\
        if ( semsPerRegion##_##regionName[x].waitFor == true )\
        {\
            sem_post( &semsPerRegion##_##regionName[x].sem );\
        }\
    };\
    pthread_mutex_unlock( &maxSpacesForRegion##_##regionName[ spaceNumber ] );\

/*
    @Summary: Similar al macro DO, pero se utiliza cuando no se va avisar a los hilos que esperan en la región.
    @Param regionName: Nombre de la region, debe ser texto. Preferiblemente una definición de código
    @Param spaceNumber: Número del espacio de la región a utilizar.
    @Ṕaram Code: Literalmente el código a ejecutar en la región.
*/
#define DO_NC(regionName,spaceNumber,Code)\
    Code \
    pthread_mutex_unlock( &maxSpacesForRegion##_##regionName[ spaceNumber ] );\


#endif // REGION_H
