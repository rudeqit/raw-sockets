## RAW sockets

Учебный проект по работе с "сырыми сокетами". Цель - понять, как
формируются заголовки пакета в стеке tcp/ip на каждом уровне.
Транcпорт - UDP. Каждый уровень вынесен в отдельную папку. Логическая
структура проекта следующая:

```
    * transport_layer (udphdr)
    * internet_layer (iphdr + udphdr)
    * link_layer (ethhdr + iphdr + udphdr)
```

* gcc version: 7.4.0
* Linux kernal version: 4.15.0-55-generic
