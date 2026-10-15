# Trabajo Práctico N°2 : Firmware y Arquitectura

### Completen la tabla: 

Usando el test ``test_fw_status.c`` como base completen la siguiente tabla, donde en cada iteración tienen que modificar el FW, el compilador o el HW.

Asegurense que en ``fw/Makefile`` estemos compilado el fw que queremos. Ejecuten un ``make clean`` antes de pasar a la siguiente línea de la tabla. 

| Caso (test_fw_status.c)  | Flag march (fw/Makefile) | Config de RTL (top.v)  | (*)Tamaño de FW | Ciclos totales | (**)Uso de celdas | Accesos a memoria |
| ------------------------ | ------------------------ | ------------------- | ---------- | ----------------- | -------------- | ---------- |
| Original                 | rv32i                    | original            |            |                   |                |            |
| Aumentar iteraciones x10  | rv32i                    | original            |            |                   |                |            |
| Anterior                 | rv32ic                   | original            |            |                   |                |            |
| Anterior                 | rv32ic                   | COMPRESSED_ISA(1)   |            |                   |                |            |
| Agregar multiplicaciones      | rv32ic                   | anterior            |            |                   |                |            |
| Anterior                 | rv32imc                  | anterior            |            |                   |                |            |
| Anterior                 | rv32imc                  | ENABLE_MUL(1)       |            |                   |                |            |
| Anterior                 | rv32imc                  | ENABLE_FAST_MUL(1)  |            |                   |                |            | 

(*)  utilizar dirección de la última instrucción

(**) estimación usando tamaño de compilación de verilator: ``du obj_dir/``

Envien la tabla completada a scian@marvell.com más una breve descripción de que se observa en cada cambio y alguna conclusión (entiendase: motivos por los cuales cambian los parámetros, situaciones en las cuales puedo preferir una configuración u otra, ¿por qué algunas configuraciones no andan?, etc). No quiero una master class y no quiero el output de una IA.
