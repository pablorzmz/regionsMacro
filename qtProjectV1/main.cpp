#include <iostream>
#include "region.h"

using namespace std;

int chancho = 500;
int nH = 0;
int nO = 0;
int fH = 0;
int fO = 0;

//#define CHANCHO
#define regionR
#define AGUA
#define regionS

#ifdef CHANCHO
    INIT_REGION( regionR,2,1 )
#endif

#ifdef AGUA
    INIT_REGION( regionS,4,4 )
#endif


#define numHilos 200
sem_t consola;
pthread_t hiloNuevo[numHilos];
void printControlled( std::string msg );
int status = 0;

/*
    Espera por la ejecución de todos los demás hilos.
    Consecuentemente, si el hilo se atrapa en la región, la ejecución no termina nunca.
*/
void esperarHilos()
{
    for( int x = 0; x < numHilos; ++x )
    {
        pthread_join( hiloNuevo[x], (void**)& status );
    }
}

#ifdef CHANCHO
    void* meterAlChancho( void* cantidad )
    {
        REGION ( regionR,0,chancho )

        DO ( regionR, 0,
               long dummy = (long)cantidad;
               std::string message = "METER AL CHANCHO $";
               message += to_string( dummy );
               message += "\n\tEl chancho actualmente tiene ";
               message += to_string( chancho );
               message += "\n\tChancho ahora tiene ";
               message += to_string( chancho + dummy );
               message += "\n\n";
               printControlled( message );
               chancho+=( dummy );
           )

        return NULL;
    }
#endif

#ifdef CHANCHO
    void* sacarDelChancho(void* cantidad)
    {
        long dummy = (long)cantidad;
        cout<<"SACAR DEL CHANCHO( $"<<chancho<<" ): $"<<dummy<<" Se va a poder ? "<<(chancho>=dummy?"true":"false")<<endl;

        REGION( regionR,1,chancho ) WHEN ( regionR,1,chancho>=dummy )
        DO_NC ( regionR,1,
                   std::string message = "\tEl chancho actualmente tiene ";
                   message +=  to_string(chancho);
                   message += "\n\tUn hilo logra sacar ";
                   message += to_string(dummy);
                   message += " del chancho\n\n";
                   printControlled(message);
                   chancho-=(dummy);
               )

        return NULL;
    }
#endif


#ifdef AGUA
    void* O(void* id)
    {
        std::string title = "O:: Oxigenos= " + to_string(nO) + " Hidrógenos= " + to_string(nH) + "\n";
        printControlled(title);
        REGION ( regionS,0,nH,nO,fH,fO )
        DO ( regionS,0,
            nO++;
           )

        REGION ( regionS,1,nH,nO,fH,fO ) WHEN ( regionS,1, (fO > 0) || (nO > 0 && nH > 1) )
        DO(regionS,1,
           if ( fO > 0)
              {
                fO--;
              }
           else
              {
               std::string msg = "\tOxigenos= " + to_string(nO) + " Hidrógenos= " + to_string(nH) + "\n";
               msg += "\tSoy el oxígeno n°: ";
               msg += to_string((long)id);
               msg += " y estoy \n";
               msg +="\t\tHaciendo agua...\n\n";
               printControlled(msg);
               nO--;
               nH -= 2;
               fH = 2;
              }
           )
        return NULL;
    }

    void* H( void* id )
    {
        std::string title = "O:: Oxigenos= " + to_string(nO) + " Hidrógenos= " + to_string(nH) + "\n";
        printControlled(title);
        REGION ( regionS,2,nH,nO,fH,fO )
        DO ( regionS,2,
            nH++;
           )

        REGION ( regionS,1,nH,nO,fH,fO) WHEN (regionS,1, (fH > 0) || (nO > 0 && nH > 1) )
        DO ( regionS,1,
             if( fH > 0 )
               {
                 fH--;
               }
               else
               {
                 std::string msg = "\tOxigenos= " + to_string(nO) + " Hidrógenos= " + to_string(nH) + "\n";
                 msg += "\tSoy el hidrógeno n°: ";
                 msg += to_string((long)id);
                 msg += " y estoy \n";
                 msg +="\t\tHaciendo agua...\n\n";
                 printControlled(msg);
                 nO--;
                 nH -= 2;
                 fH = 1;
                 fO = 1;
               }
           )
        return NULL;
    }

#endif

/*
    @Param msg: Mensaje a mostrar de manera controlada por la consola
*/

void printControlled( std::string msg )
{
    sem_wait( &consola );
    cout<<msg;
    sem_post( &consola );
}


int main()
{

sem_init(&consola,0,1);
pthread_attr_t at;
pthread_attr_init(&at);
pthread_attr_setdetachstate( &at,PTHREAD_CREATE_JOINABLE );
//long status = 0;

#ifdef CHANCHO

    SHARED( regionR,chancho )

    long cantidadPedir = 0;
    long cantidadMeter = 0;

    for( int x = 0; x < numHilos; ++x )
    {
        cantidadPedir = 200 + rand()%1000;
        cantidadMeter = 200 + rand()%900;

        if (x%2 == 0)
        {
            pthread_create(&hiloNuevo[x],&at,sacarDelChancho,(void*)cantidadPedir);
        }else
        {
            pthread_create(&hiloNuevo[x],&at,meterAlChancho,(void*)cantidadMeter);
        }

    }    
    pthread_attr_destroy( &at );
    //esperarHilos();
#endif

#ifdef AGUA

    SHARED(regionS,nH)
    SHARED(regionS,nO)
    SHARED(regionS,fH)
    SHARED(regionS,fO)

    for ( long id = 0; id < numHilos; ++ id )
    {
        if ( id%2 == 0  )
        {
           pthread_create(&hiloNuevo[id],&at,H,(void*)(id+1));
        }else
        {
           pthread_create(&hiloNuevo[id],&at,O,(void*)(id+1));
        }
    }

    pthread_attr_destroy( &at );
    //esperarHilos();

#endif

printControlled("Ejecución correcta\n");
#ifdef CHANCHO
    DESTROY_REGION( regionR )
#endif

#ifdef AGUA
    DESTROY_REGION( regionS )
#endif

    return 0;
}

