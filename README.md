# Matching Engine

Matching engine simples, em memória, para um único ativo, implementada em **C++17**.

O projeto suporta ordens **limit**, **market** e **pegged**, prioridade preço-tempo, cancelamento, alteração de ordens e visualização do livro de ofertas.

## Compilação e execução

O projeto utiliza apenas a biblioteca padrão do C++ e não possui dependências externas.

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -Iinclude -o matching_engine
```

Linux/macOS:

```bash
./matching_engine
```

Windows com MinGW/Git Bash:

```bash
./matching_engine.exe
```

Dentro do programa, `help` mostra todos os comandos disponíveis.

## Comandos

```text
limit buy <price> <qty>
limit sell <price> <qty>

market buy <qty>
market sell <qty>

peg bid buy <qty>
peg offer sell <qty>

cancel order <order_id>

modify order <order_id> <price> <qty>
modify peg <order_id> <qty>

print book
print book aggregated

help
exit
```

Em `modify order`, preço e quantidade devem sempre ser informados. Para alterar apenas um deles, basta repetir o valor atual do outro.

Exemplo:

```text
>>> limit buy 10.00 100

Order created: buy 100 @ 10.00 order_1

>>> modify order order_1 9.98 100

Order modified

>>> print book

Ordens de Compra    | Ordens de Venda
--------------------|--------------------
100 @ 9.98          |
```

## Regras e decisões de implementação

| Situação | Comportamento adotado |
|---|---|
| Prioridade | Melhor preço primeiro; dentro do mesmo preço, FIFO. |
| Execução parcial | A quantidade restante mantém sua prioridade original. |
| Limit order que cruza o livro | É executada imediatamente. A sobra, se houver, permanece no livro. |
| Preço do trade | Preço da ordem **resting**, isto é, da ordem que já estava no livro. |
| Market order sem liquidez suficiente | A quantidade não executada é descartada e nunca permanece no livro. |
| Alteração de preço | A ordem perde prioridade e recebe uma nova prioridade temporal. |
| Redução de quantidade | Mantém prioridade. |
| Aumento de quantidade | Perde prioridade e vai para o fim da fila daquele preço. |
| `peg bid` | Ordem de compra que acompanha o melhor bid. |
| `peg offer` | Ordem de venda que acompanha o melhor offer. |
| Repricing automático de peg | Preserva a prioridade temporal original. |
| Peg sem preço de referência | Recebe ID, fica inativa fora do livro e é ativada quando a referência passa a existir. |

Crossing limit orders são executadas porque o preço limite é tratado como o pior preço aceito pela ordem. Assim, se já existir contraparte disponível em preço igual ou melhor, a execução pode ocorrer imediatamente e o livro não permanece cruzado.

Internamente, cada execução contra uma ordem resting gera um `Trade` separado. Na saída da CLI, trades consecutivos realizados no mesmo preço são agregados para seguir o formato do enunciado.

Todas as ordens submetidas recebem um `order_id` monotonicamente crescente, inclusive market orders transitórias.

### Pegged orders

Uma pegged order ativa também participa do cálculo do próprio best bid ou best offer. Portanto, se a ordem que originalmente definia a referência desaparecer, mas a peg continuar naquele melhor preço, ela permanece nesse nível.

Pegged orders podem ser canceladas normalmente. Sua quantidade pode ser alterada com `modify peg`: redução mantém prioridade e aumento perde prioridade. O preço não pode ser alterado manualmente, pois é definido pela referência.

## Estruturas de dados

O livro utiliza `std::map<int, PriceLevel, std::greater<int>>` para bids, `std::map<int, PriceLevel>` para offers e `std::unordered_map<unsigned long long, OrderNode*>` para busca direta por identificador.

Cada `PriceLevel` contém uma **lista duplamente ligada** customizada. O primeiro nó é a ordem mais antiga daquele preço e, portanto, a próxima a ser executada. A lista também permite remover diretamente uma ordem conhecida durante cancelamentos, modificações e repricing.

O `std::unordered_map` permite busca média O(1) por `order_id`, enquanto os `std::map` mantêm os níveis de preço ordenados para acesso ao melhor bid e melhor offer.

O `OrderBook` é responsável pela vida útil dos `OrderNode` criados dinamicamente. Os `PriceLevel` apenas mantêm referências para esses nós.

### Representação de preços

Os preços são armazenados internamente como **centavos inteiros**, e não como `float` ou `double`:

```text
10.00 -> 1000
10.10 -> 1010
9.99  -> 999
```

Isso evita problemas de comparação de ponto flutuante e define um tick de preço de `0.01`. A CLI aceita preços positivos com no máximo duas casas decimais e quantidades inteiras positivas.

Como simplificação, a atualização de pegged orders percorre o índice de ordens, e a reinserção por prioridade pode exigir percorrer a fila do `PriceLevel`. Essa escolha é aceitável dentro do escopo do desafio, que não exige otimizações de escalabilidade.

## Visualização do livro

`print book` mostra as ordens individualmente, preservando a ordem dentro de cada nível de preço.

`print book aggregated` soma as quantidades das ordens presentes no mesmo preço.

## Testes

Os testes utilizam apenas `assert` da biblioteca padrão do C++, sem frameworks externos. Eles cobrem as estruturas do livro e os principais comportamentos da engine, incluindo FIFO, matching, partial fills, cancelamento, modificações, pegged orders e entradas inválidas.

A visualização do livro é conferida manualmente e o parser da CLI não possui testes automatizados.

### PriceLevel

```bash
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_price_level.cpp src/price_level.cpp -Iinclude -o test_price_level
./test_price_level
```

### OrderBook

```bash
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_order_book.cpp src/order_book.cpp src/price_level.cpp src/utils.cpp -Iinclude -o test_order_book
./test_order_book
```

### MatchingEngine

```bash
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_matching_engine.cpp src/matching_engine.cpp src/order_book.cpp src/price_level.cpp src/utils.cpp -Iinclude -o test_matching_engine
./test_matching_engine
```

## Escopo e limitações

O projeto segue deliberadamente o escopo proposto no desafio: trabalha com apenas um ativo, mantém todo o estado em memória e não possui persistência, rede, conexão com exchange externa, concorrência ou multithreading.

A CLI valida os argumentos obrigatórios, preços, quantidades e identificadores. Tokens adicionais colocados após um comando já válido são atualmente ignorados pelo parser.
