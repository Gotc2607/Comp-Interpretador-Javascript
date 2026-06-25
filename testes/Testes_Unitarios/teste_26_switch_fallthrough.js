let x = 2;
let y = 0;
switch (x) {
    case 1:
        y += 1;
    case 2:
        y += 10;
    case 3:
        y += 100;
        break;
    case 4:
        y += 1000;
}
console.log(y);
