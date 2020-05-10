# FEUP-SOPE-Proj2 - Readme - Proj1

### Duração entre pedidos
A duração entre os pedidos é random entre os 50 ms e os 100 ms

### Duração dos pedidos 
A duração dos pedidos é random entre 200 ms e 1000 ms

### Duração do servidor
O servidor fica ativo durante n segundos sendo n o argumento da flag -t. É usado um sinal alarm com duração n que notifica quando o tempo acaba.

### Mecanismos de sincronização
São utilizados 2 semáforos para sincronização durante os pedidos. 
No 1º semáforo, o servidor faz wait à espera que o cliente crie o fifo privado, sendo desbloqueado pela thread cliente quando esta abre o seu fifo privado.
No 2º semáforo, o cliente faz wait à espera de uma resposta do servidor, sendo desbloqueado pela thread do servidor que responde ao seu pedido, depois desta fazer write.

### Threads servidor
Estas threads fazem sleep da duração do request nos casos em que o aceita, de modo a dar TIMUP quando este tempo acabar. Definimos 500,000 como número máximo de threads.

### Gestão de memória
Em todos os casos de saída do programa, todos os fifos e semáforos são fechados e removidos, ou seja, todos os recursos são devolvidos ao sistema.

# FEUP-SOPE-Proj2 - Readme - Proj2

### Alterações ao projeto 1
Foi corrigido o problema dos printf's quando 2 threads o faziam ao mesmo tempo. A solução foi adicionar um mutex lock antes dos prints e unlock depois para garantir que são feitas 1 de cada vez entre threads.
A duração entre pedidos foi diminuída de 50-100 ms para 30-50 ms.
Foi removido o segundo semáforo da 1ª parte uma vez que o programa "bloqueia" no read. O primeiro semáforo foi mantida como uma segurança uma vez que não é garantido que o cliente crie o seu fifo privado antes do servidor o tentar abrir.
O pthread join no ficheiro U1 foi removido, sendo substituídos por detach.

### Mecanismos de sincronização
Foram adicionados 3 semáforos, 1 deles para os printf como referido anteriormente. O segundo semáforo (placesMut) é utilizado no ficheiro Q por 2 vezes. Numa primeira instâcia é usado para garantir que só 1 thread de cada vez atribui um lugar a um cliente, sendo que foi adicionada uma condition variable para quando o número de lugares disponíveis é 0. Quando os pedidos são atendidos é realizado um pthread_cond_signal(), que nada faz caso nenhum dos pedidos esteja em wait, para sinalizar que 1 lugar ficou disponível. Devido ao modo de funcionamento destes semáforos, garantimos assim que são atendidos primeiro os pedidos que foram realizados primeiro, mantendo-se a ordem de chegada. Numa segunda instância, é utilizado para garantir que só 1 thread liberta o seu lugar de cada vez. O terceiro semáforo (semThreads) é utilizado para sincronizar as threads, isto é, é inicializado com o valor da flag -n, sendo reduzido por 1 sempre que recebe 1 mensagem. Caso este valor atinja 0, fica preso no wait à espera que uma thread acabe., não realizando reads. Sempre que 1 thread retorna, é feito sem_post, sinalizando que mais 1 thread está disponível, libertando 1 das threads que está no wait de modo a atender o seu pedido. 

### Escolha dos lugares
Optámos por utilizar uma implementação de uma queue (//https://www.geeksforgeeks.org/queue-linked-list-implementation/) que no início do servidor é preenchida com int's, representando os numeros lugares disponíveis (Ex: -l 3 faz com que a queue seja {1,2,3}). Ao utilizar um lugar a thread retira o primeiro int da queue e utiliza-o. Quando o pedido dá TIMUP, esse int é reposto na queue.

### Valores por omissão
Por omissão o valor de nThreads e nPlaces é 5.
