let x = 2;
let y = 0;
switch (x) {
    case 1: {
        y = 10;
        break;
    }
    case 2: {
        y = 20;
        break;
    }
    default: {
        y = 99;
    }
}
console.log(y);