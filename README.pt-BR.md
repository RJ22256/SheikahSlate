# Sheikah Slate

[English](README.md) | **Português**

*Breath of the Wild, recompilado, na prancheta em suas mãos.*

Sheikah Slate é um porte do [BOTWRecompAndroid](https://github.com/mamcjr/BOTWRecompAndroid) (0.2.8-beta) feito
especificamente para o **Samsung Galaxy Tab S10 Ultra** (SM-X920 / SM-X926B / SM-X920N / SM-X926N). Como o original, é
uma recompilação estática de *The Legend of Zelda: Breath of the Wild* (Nintendo Switch, versões 1.0.0, 1.6.0, 1.8.2 e 1.9.0):
não é emulação. O código ARM64 do jogo vira C compilado para o aparelho, e os gráficos NVN são traduzidos para Vulkan.

Ele é instalado como um app próprio (`com.sheikahslate.tab`) e pode ficar ao lado do original.

> [!WARNING]
> **Beta.** O trabalho para o tablet foi escrito e conferido a partir do código e ajustado com logs enviados de um Tab
> S10 Ultra (29-30 fps no mundo na 0.1.9-slate.8); esta versão ainda não rodou num. Por favor, relate o que encontrar:
> [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md) tem uma lista de testes (em inglês).

> [!NOTE]
> **Nenhum arquivo do jogo está incluído.** Você precisa do seu próprio dump obtido legalmente de *Breath of the Wild*
> 1.0.0, 1.6.0, 1.8.2 ou 1.9.0 (`exefs` e `romfs`, extraídos). Instruções de como obtê-lo não serão fornecidas.

## O que muda em relação ao original

Tudo o que a 0.2.8 do original tem está aqui - importar e exportar saves (um .zip, ou a pasta de saves de um emulador:
tela inicial > Configurações > **Saves**), a DLC, os arquivos do jogo usados no lugar, Sync lag, cutscenes a 60 fps, onze
idiomas no menu - e por cima o trabalho do tablet:

- **Predefinição gráfica Tab S10 Ultra** (Vídeo), feita com os logs do próprio tablet: 30 fps no ritmo da tela de 120 Hz,
  cena de 900p estabilizada, FSR 1 para 1080p. É o padrão no tablet.
- **Ritmo uniforme dos quadros:** cada quadro fica exatamente 4 atualizações na tela a 30 fps, em vez de 3, 4 ou 5.
- **Mali:** decalques leem o G-buffer da memória de tiles da GPU, passadas de renderização sobrepostas, matemática de
  shader de 32 bits automática, resolução do jogo estabilizada, e uma captura de desempenho completa no menu.

- **Orientação:** o app se declara um jogo (`appCategory="game"`), então a trava em paisagem continua valendo no
  Android 16 (One UI 8+), que ignora `sensorLandscape` em telas grandes para apps com API 36.
- **Tela 16:10:** com "Livre" na 1.8.2, o original deixava o mundo esticado ~11% na vertical; agora a imagem fica em
  16:9, ocupando a largura, com faixas de 92 px em cima e embaixo (o notch da câmera fica numa delas).
- **Notch da câmera:** a janela vai por baixo dele em qualquer versão do Android, e os controles na tela saem de baixo
  dele, também com o tablet virado.
- **Controles na tela:** em telas grandes, medidos em milímetros (botões de ~14 mm, perto das bordas); celulares sem
  mudança.
- **Taxa de atualização:** Vídeo > Taxa de atualização da tela: Máxima (120 Hz), 60 Hz ou Sistema.
- **Memória da GPU Mali:** a importação do heap do jogo é testada ao iniciar; se o driver recusar, o app usa as cópias
  pela CPU desde o começo, em vez de shaders lendo zeros.
- **Nome e ícone:** Sheikah Slate, com um ícone original.

Detalhes: [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md). Como compilar e os créditos completos: [README.md](README.md).
O README original: [docs/upstream/README.pt-BR.md](docs/upstream/README.pt-BR.md).

*The Legend of Zelda*, *Breath of the Wild* e *Sheikah* são marcas da Nintendo. *Galaxy* e *Galaxy Tab* são marcas da
Samsung. Este projeto não tem afiliação nem endosso da Nintendo ou da Samsung.
