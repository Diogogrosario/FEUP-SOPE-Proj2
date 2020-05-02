# FEUP-SOPE-Proj2 - Readme

### Duração entre pedidos
A duração entre os pedidos é random entre os 50 ms e os 100 ms

### Duração dos pedidos 
A duração dos pedidos é random entre 200 ms e 1000 ms

### Duração do servidor
O servidor fica ativo durante n segundos sendo n o argumento da flag -t

### Mecanismos de sincronização
São utilizados 2 semáforos para sincronização durante os pedidos. 
No 1º semáforo, o servidor faz wait à espera que o cliente crie o fifo privado, sendo desbloqueado pela thread cliente quando esta abre o seu fifo privado.
No 2º semáforo, o cliente faz wait à espera de uma resposta do servidor, sendo desbloqueado pela thread do servidor que responde ao seu pedido, depois desta fazer write.

### Threads servidor
Estas threads fazem sleep da duração do request, de modo a dar TIMUP quando este tempo acabar.
