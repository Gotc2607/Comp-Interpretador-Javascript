let x = 5;
let y = 0;
switch (x) {
    case 1:
        y += 1;
        break;
    default:
        y += 10;
    case 3:
        y += 100;
        break;
    case 4:
        y += 1000;
}
console.log(y);
