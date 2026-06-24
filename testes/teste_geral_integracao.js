console.log("=== TESTE GERAL DE INTEGRACAO ===");

console.log("1. Declaracoes e Nullish:");
let a = 10;
const str = "Interpretador";
var c;
c ??= 42;
console.log(a);
console.log(str);
console.log(c);

console.log("2. Matematica e ++/--:");
a += 5;
a++;
let d = (a * 2) % 10;
let e = 2 ** 3;
console.log(a);
console.log(d);
console.log(e);

console.log("3. Logica e Comparacao:");
if (a === 16 && str !== "Erro" || d < 0) {
    console.log(1);
} else {
    console.log(0);
}
console.log(!0);

console.log("4. Switch/Case:");
switch (d) {
    case 1:
        console.log(100);
        break;
    case 2:
        console.log(200);
        break;
    default:
        console.log(300);
}

console.log("5. Laco For (Break/Continue):");
let somaLaco = 0;
let i = 0;
for (i = 0; i < 5; i++) {
    if (i === 1) {
        continue;
    }
    if (i === 4) {
        break;
    }
    somaLaco += i;
}
console.log(somaLaco);

console.log("6. Arrays e Indexacao de String:");
let arr;
arr[0] = 50;
arr[1] = 50;
arr[2] = arr[0] + arr[1];
console.log(arr[2]);
console.log(str[0]);

console.log("7. Funcoes e Escopo:");
let globalVar = 99;

function fibonacci(n) {
    let globalVar = 0;
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

console.log(fibonacci(6));
console.log(globalVar);

console.log("8. Concatencacao de Strings (+):");
console.log("a" + "b");
console.log("Val: " + 42);
console.log(1 + "!");

console.log("9. Escopo de Bloco (Shadowing):");
let shadowVar = 10;
{
    let shadowVar = 20;
    console.log(shadowVar);
}
console.log(shadowVar);

console.log("10. Switch com Fallthrough:");
let swVal = 2;
let swOut = 0;
switch (swVal) {
    case 1:
        swOut += 1;
    case 2:
        swOut += 10;
    case 3:
        swOut += 100;
        break;
    default:
        swOut += 1000;
}
console.log(swOut);

console.log("=== FIM DO TESTE ===");