# Sheikah Slate

[English](README.md) | **Português**

*Breath of the Wild, recompilado, na prancheta em suas mãos.*

Sheikah Slate é um porte do [BOTWRecompAndroid](https://github.com/mamcjr/BOTWRecompAndroid) (0.1.9-beta) feito
especificamente para o **Samsung Galaxy Tab S10 Ultra** (SM-X920 / SM-X926B / SM-X920N / SM-X926N). Como o original, é
uma recompilação estática de *The Legend of Zelda: Breath of the Wild* (Nintendo Switch, versões 1.0.0, 1.6.0 e 1.8.2):
não é emulação. O código ARM64 do jogo vira C compilado para o aparelho, e os gráficos NVN são traduzidos para Vulkan.

Ele é instalado como um app próprio (`com.sheikahslate.tab`) e pode ficar ao lado do original.

> [!WARNING]
> **Não testado no aparelho a que se destina.** As mudanças foram escritas e conferidas a partir do código (compilação e
> testes unitários num PC), não num Tab S10 Ultra. O original foi testado num celular com GPU Adreno; a Immortalis-G720
> do Tab é uma GPU Mali e segue os caminhos mais lentos e compatíveis do motor. Espere bugs e, por favor, relate o que
> encontrar: [docs/TAB-S10-ULTRA.md](docs/TAB-S10-ULTRA.md) tem uma lista de testes (em inglês).

> [!NOTE]
> **Nenhum arquivo do jogo está incluído.** Você precisa do seu próprio dump obtido legalmente de *Breath of the Wild*
> 1.0.0, 1.6.0 ou 1.8.2 (`exefs` e `romfs`, extraídos). Instruções de como obtê-lo não serão fornecidas.

## O que muda em relação ao original

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
