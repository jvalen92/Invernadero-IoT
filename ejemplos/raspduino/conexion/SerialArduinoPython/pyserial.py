import serial
import time



# Se establece la conexion
ser = serial.Serial('/dev/ttyACM0',9600, timeout=1)
time.sleep(1)


# Se clasifica la intruccion, teniendo dos categorias
# 'a' para actuadores, modificando las variables
# 's' para sensores, obteniendo el valor
def getOrder(st , value=0):
    st=st.replace('_','').replace("sise", "") 
    toSend=0
    if(st[1:]=="humedadsuelo"):
           if(st[0]=='s'):
               toSend=-1
           else:
                toSend=-11
    
    elif(st[1:]=="ph"):
            if(st[0]=='s'):
                toSend=-2
            else:
                toSend=-12
    
    elif(st[1:]=="luzinfrarroja"):
            if(st[0]=='s'):
                toSend=-3
            else:
                toSend=-13
                
    elif(st[1:]=="luzuv"):
            if(st[0]=='s'):
                toSend=-4
            else:
                toSend=-14
    
    elif(st[1:]=="luzblanca"):
            if(st[0]=='s'):
                toSend=-5
            else:
                toSend=-15
                
    return toSend
            
        
while True:

    print ("escribir orden")

    x = input()
    value=float(0)

    #obtenemos el tipo de instruccion con getOrder, y se convierte a str ya que arduino lo puede recibir correctamente

    if(x[0]=='a'):
         value = float(input())
  
    ser.write(str(getOrder(x))+';'+str(value))
    time.sleep(0.1)
    response = ser.readline()
    print response
    ser.flush()

