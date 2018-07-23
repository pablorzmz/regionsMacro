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
    sharedValues()
        :name("")        
        ,sem()
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
    for ( size_t index2 = 0; index2 < maxV##_##regionName; ++ index2 )\
        {\
            sem_destroy( &semaphores##_##regionName[ index2 ] );\
        };\


/*
    @Summary: Macro DESTROY_REGION para destruir todos los recursos creados al usar las regiones
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param maxRegionCalls: Numero máximo de espacios de región para la región actualmente a definir
    @Param maxSharedVariables: Número máximo de variables que se compartir dentro de la región
*/

#define INIT_REGION( regionName, maxRegionCalls, maxSharedVariables )\
    pthread_mutex_t maxSpacesForRegion##_##regionName[ maxRegionCalls ];\
    sem_t semaphores##_##regionName[ maxSharedVariables ];\
    void initialize##_##regionName##_##aaabbbccc()\
    {\
        for (int x = 0; x < maxRegionCalls; ++x ) \
        { \
            pthread_mutex_init(&maxSpacesForRegion##_##regionName[ x ],NULL); \
        } \
    } \
    sharedValues myValues##_##regionName[ maxSharedVariables ]; \
    static size_t maxV##_##regionName = maxSharedVariables; \
    static size_t maxR##_##regionName = maxRegionCalls; \
    static size_t indexVaribales##_##regionName = 0; \
    static bool  controlInit##_##regionName = false; \

/*
    @Summary: Macro SHARED para "compartir" una variable global en una región específica.
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param variableName: Nombre del identificador de la variable para declarar como compartida en la región
*/
#define SHARED( regionName, variableName ) \
    if ( controlInit##_##regionName == false ) \
    { \
        initialize##_##regionName##_##aaabbbccc();\
        controlInit##_##regionName = true;\
    } \
    sharedValues temp##_##variableName;\
    sem_t variableName##_##regionName;\
    sem_init( &variableName##_##regionName,0,0 );\
    semaphores##_##regionName[ indexVaribales##_##regionName ] = variableName##_##regionName;\
    temp##_##variableName.sem = variableName##_##regionName;\
    temp##_##variableName.name = #variableName;\
    myValues##_##regionName[ indexVaribales##_##regionName ] = temp##_##variableName;\
    ++indexVaribales##_##regionName;

/*
    @Summary: Macro REGION que declara un espacio a usar después de la incialización de la región como tal.
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param spaceNumber: Número del espacio de la región a utilizar.
    @Param ...: Lista de nombres de las variables definidas previamente con la macro SHARED como compar-
    tidas que puedan o no cambiar dentro de la región.

*/
#define REGION( regionName,spaceNumber,... )\
    static char constChar##_##regionName##_##spaceNumber[] = {#__VA_ARGS__};\
    static char* p##_##regionName##_##spaceNumber = strtok(constChar##_##regionName##_##spaceNumber,",");\
    static std::vector<std::string> vector##_##regionName##_##spaceNumber;\
    while(p##_##regionName##_##spaceNumber)\
    {\
        vector##_##regionName##_##spaceNumber.push_back(p##_##regionName##_##spaceNumber);\
        p##_##regionName##_##spaceNumber = strtok(NULL,",");\
    };\
    p##_##regionName##_##spaceNumber = NULL;\
    pthread_mutex_lock( &maxSpacesForRegion##_##regionName[ spaceNumber ] );\

/*
    @Summary: Macro WHEN que detiene el acceso al región crítica.
    @Param regionName: Nombre de la región, debe ser texto. Preferiblemente una definición de código.
    @Param spaceNumber: Número del espacio de la región a utilizar.
    @Param condition: Método o condición booleana para entrar a ejecutarse en la región.
*/
#define WHEN( regionName, spaceNumber, condition )\
    while ( (condition) == false ){\
        for (size_t x = 0; x <maxV##_##regionName; ++ x ){\
            for ( size_t y = 0; y < vector##_##regionName##_##spaceNumber.size(); ++y ){\
                    if ( vector##_##regionName##_##spaceNumber[y] == myValues##_##regionName[x].name )\
                        {\
                            sem_wait(&(myValues##_##regionName[x].sem));\
                        }\
                }\
          }\
    };\

/*
    @Summary: Macro DO para realizar lo que se desee cuando se entre a la región.
    @Param regionName: Nombre de la region, debe ser texto. Preferiblemente una definición de código
    @Param spaceNumber: Número del espacio de la región a utilizar.
    @Ṕaram Code: Literalmente el código a ejecutar en la región.
*/
#define DO(regionName,spaceNumber,Code)\
    Code\
    for (size_t x = 0; x <maxV##_##regionName; ++ x ){\
        for ( size_t y = 0; y < vector##_##regionName##_##spaceNumber.size(); ++y ){\
                if ( vector##_##regionName##_##spaceNumber[y] == myValues##_##regionName[x].name  )\
                    { \
                        sem_post(&(myValues##_##regionName[x].sem));\
                    }\
            }\
       }; \
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
