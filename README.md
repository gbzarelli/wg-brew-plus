# WG Brew Plus
<p align="center">
<img src="images/2017-07-30 17.58.12.jpg" width="450"/></p>

Projeto para controle/automação do processo de cerveja artesanal.

O projeto automatiza o processo de brassagem e fervura das etapas de fabricação da cerveja artesanal.

## ROTINA LOOP DO PROGRAMA

No processo de brassagem pode-se determinar a quantidade de rampas e suas determinadas temperaturas/tempo (pode-se determinar um pré aquecimento), alarmes são reproduzidos ao fim de cada rampa.

Ao fim do processo de brassagem é reproduzido um aviso sonoro e o sistema aguarda interação para o incio da etapa de fervura (nesse momento o rele de aquecimento é desligado).

O processo de fervura assim como o de brassagem é todo pré configurado, determina-se a temperatura de aquecimento e o tempo de cada lupulo e o sistema se encarrega em controlar a temperatura de fervura e reproduzir os alarmes no inicio do tempo configurado para cada lupulo.

```xml
   1 - BRASSAGEM
   tempo:    |--------Tn--------|------|T rampa 1|------|T rampa 2|  ...
   temp:     0º- > - > - > - > 50º - > 60º------60º - > 70º------70º ...
   timeline: /-pre-aquecimento-/------/--rampa1--/------/--rampa2--/-...
                               |                 |                 |
                               |-> Soa alarme ao fim dos t/pre e rampa.

   -> Ao chegar no fim do tempo da ultima rampa o sistema aguarda confirmação
   para iniciar fervura, enquanto não tem o 'ok' o rele de aquecimento é deslig.

   2 - FERVURA
   tempo:    |---------Tn--------|---------------T fervura--------------|
   temp:     xº - > - > - > - > 100º-----------------------------------100º
   timeline: /-aquecimento ferv-/------/-lup1-/--/-lup2-/-------/-lup3-/-
                                       |         |              |
                                       |->Soa alarme ao iniciar tempo de lupo;
```

# Board

<img src="images/Screenshot from 2019-12-30 11-34-49.png" width="450">
<img src="images/2017-05-12 22.57.16.jpg" width="450"/>

# Informações para contato

- Guilherme Biff Zarelli - Analista Desenvolvedor
  - gbzarelli@helpdev.com.br
  - (16) 98805-6630
  
- Willian Miguel Hrycyki - Engenheiro de Controle e Automação
  - willianshrycyki@gmail.com
  - (16) 98111-6803
