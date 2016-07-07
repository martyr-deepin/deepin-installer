var
    fileCount = 0;
    canvas = null,
    stage  = null,
    stage_progress = 0;
    manifest = [
        {src:'assets/img/action.png', id:'action'},
        {src:'assets/img/background1.png', id:'background1'},
        {src:'assets/img/background2.png', id:'background2'},
        {src:'assets/img/background3.png', id:'background3'},
        {src:'assets/img/background5_1.png', id:'background5_1'},
        {src:'assets/img/background5_2.png', id:'background5_2'},
        {src:'assets/img/chart.png', id:'chart'},
        {src:'assets/img/computer.png', id:'computer'},
        {src:'assets/img/icon1.png', id:'icon1'},
        {src:'assets/img/icon2.png', id:'icon2'},
        {src:'assets/img/icon3.png', id:'icon3'},
        {src:'assets/img/icon4.png', id:'icon4'},
        {src:'assets/img/icon5.png', id:'icon5'},
        {src:'assets/img/icon6.png', id:'icon6'},
        {src:'assets/img/logo1.png', id:'logo1'},
        {src:'assets/img/logo2.png', id:'logo2'},
        {src:'assets/img/logo3.png', id:'logo3'},
        {src:'assets/img/logo4.png', id:'logo4'},
        {src:'assets/img/people1_2.png', id:'people1_2'},
        {src:'assets/img/people2_3.png', id:'people2_3'},
        {src:'assets/img/people4_5.png', id:'people4_5'},
        {src:'assets/img/people5_5.png', id:'people5_5'},
        {src:'assets/img/people6_5.png', id:'people6_5'},
        {src:'assets/img/table.png', id:'table'}
    ],
    loader = null,
    COMMON = {};

COMMON.screen = {
    x:175,
    y:34,
    width:401,
    height:231
}



function assetLoader(id){
    return new createjs.Bitmap(loader.getResult(id));
}

function getWidthCenter(obj){
    return (stage.canvas.width - obj.image.width) / 2;
}

function getHeightCenter(obj){
    return (stage.canvas.height - obj.image.height) / 2;
}

function SpriteMask(x, y, width, height){
    var mask = new createjs.Shape();
    mask.graphics.beginFill("#000").drawRect(0, 0, width, height);
    mask.x = x;
    mask.y = y;
    mask.alpha = 0;
    return mask;
}

function Stage1(){
    stage_progress = 1;
    console.debug("=================stage_progress" + stage_progress);
    var computer = new assetLoader('computer');
    computer.x = getWidthCenter(computer);
    computer.y = 50;
    computer.alpha = 0;
    stage.addChild(computer);

    COMMON.computer = computer;

    var background1 = new assetLoader('background1');
    background1.x = COMMON.screen.x;
    background1.y = COMMON.screen.y;
    background1.alpha = 0;
    stage.addChild(background1);

    var logo1 = new assetLoader('logo1');
    logo1.x = getWidthCenter(logo1);
    logo1.y = 68;
    logo1.alpha = 0;
    stage.addChild(logo1);

    var table = new assetLoader('table');
    table.x = getWidthCenter(table);
    table.y = getHeightCenter(table) + 55;
    table.alpha = 1;
    stage.addChild(table);

    var icon1 = new assetLoader('icon1'),
    iconAlign = {
        x: getWidthCenter(icon1) - 135,
        y: getHeightCenter(icon1) + 48
    };
    icon1.x = iconAlign.x - 15;
    icon1.y = iconAlign.y;
    icon1.alpha = 0;
    stage.addChild(icon1);

    var icon2 = new assetLoader('icon2');
    icon2.x = icon1.x + icon1.image.width + 10;
    icon2.y = iconAlign.y;
    icon2.alpha = 0;
    stage.addChild(icon2);

    var icon3 = new assetLoader('icon3');
    icon3.x = icon2.x + icon1.image.width + 10;
    icon3.y = iconAlign.y;
    icon3.alpha = 0;
    stage.addChild(icon3);

    var icon4 = new assetLoader('icon4');
    icon4.x = icon3.x + icon1.image.width + 10;
    icon4.y = iconAlign.y;
    icon4.alpha = 0;
    stage.addChild(icon4);

    var icon5 = new assetLoader('icon5');
    icon5.x = icon4.x + icon1.image.width + 10;
    icon5.y = iconAlign.y;
    icon5.alpha = 0;
    stage.addChild(icon5);

    var icon6 = new assetLoader('icon6');
    icon6.x = icon5.x + icon1.image.width + 10;
    icon6.y = iconAlign.y;
    icon6.alpha = 0;
    stage.addChild(icon6);

    var chart = new assetLoader('chart');
    chart.x = -chart.image.width;
    chart.y = stage.canvas.height - chart.image.height;
    stage.addChild(chart);

    //var shadow1 = new createjs.Shape();
    //shadow1.graphics
    //.beginLinearGradientFill(["#ce5e1b","#ce5e1b"], [0, 1], 100, 50, 230, -50)
    //.moveTo(0,0)
    //.bezierCurveTo(200,-40,220,10,230,20)
    //.bezierCurveTo(200,20,194,98,196,102)
    //// .lineTo(0,105)
    //.lineTo(0,0);
    //shadow1.alpha = 0.45;
    //shadow1.x = 56;
    //shadow1.y = 86;
    //stage.addChild(shadow1);

    var mask1 = new SpriteMask(
        table.x,
        table.y - table.image.height,
        table.image.width,
        table.image.height
    );
    stage.addChild(mask1);

    var mask2 = new SpriteMask(230,-230,300,300);
    mask2.rotation = -35;
    stage.addChild(mask2);

    var maskScreen = new SpriteMask(
        COMMON.screen.x,
        COMMON.screen.y,
        COMMON.screen.width,
        COMMON.screen.height
    );
    stage.addChild(maskScreen);

    table.mask = mask1;
    //shadow1.mask = mask2;
    background1.mask = maskScreen;

    COMMON.maskScreen = maskScreen;

    var textBox1_Show = function(){
        var tl = new TimelineMax();
        tl.to('#text1', 0, {alpha:0, scale:0.1, rotation:175, x:"+=135", y:"+=10"},0)
        .to('#text1', 0.8, {alpha:0.9, scale:1, rotation:0, x:"-=135",y:"-=10"},0)
        .to(mask2, 0.8, {x:0, y:0},0);
        return tl;
    }

    var textBox1_Exit = function(){
        var tl = new TimelineMax();
        tl.to('#text1', 0.8, {alpha:0, scale:0.1, rotation:175, x:"+=135", y:"-=20"},0)
        .to(mask2, 1, {x:230, y:-230},0);
        return tl;
    }

    var tl = new TimelineMax();
    tl.to(computer, 0, {alpha:1, y:0})
    .to(background1, 0.8, {alpha:1})
    .to(logo1, 0.8, {alpha:1, delay:0.4})
    .to(mask1, 0.8, {y:"+="+table.image.height})
    .to(icon1, 0.3, {alpha:1, y:iconAlign.y - 16, delay:0.1})
    .to(icon2, 0.3, {alpha:1, y:iconAlign.y - 16, delay:0.1})
    .to(icon3, 0.3, {alpha:1, y:iconAlign.y - 16, delay:0.1})
    .to(icon4, 0.3, {alpha:1, y:iconAlign.y - 16, delay:0.1})
    .to(icon5, 0.3, {alpha:1, y:iconAlign.y - 16, delay:0.1})
    .to(icon6, 0.3, {alpha:1, y:iconAlign.y - 16, delay:0.1})
    .add(textBox1_Show());

    tl.add(textBox1_Exit(),"+="+3)
    .to(icon1, 0.3, {alpha:0, y:iconAlign.y - 32, delay:0.1})
    .to(icon5, 0.3, {alpha:0, y:iconAlign.y - 32, delay:0.1})
    .to(icon2, 0.3, {alpha:0, y:iconAlign.y - 32, delay:0.1})
    .to(icon6, 0.3, {alpha:0, y:iconAlign.y - 32, delay:0.1})
    .to(icon4, 0.3, {alpha:0, y:iconAlign.y - 32, delay:0.1})
    .to(icon3, 0.3, {alpha:0, y:iconAlign.y - 32, delay:0.1})
    .to(mask1, 0.8, {y:"-="+table.image.height})
    .to(logo1, 0.8, {alpha:0})
    .to(background1, 0.8, {alpha:0});

    return tl;
}

function Stage2(){
    stage_progress = 2;
    console.debug("=================stage_progress" + stage_progress);
    var background2 = assetLoader('background2');
    background2.x = 175;
    background2.y = 34;
    background2.alpha = 0;
    stage.addChild(background2);

    var logo2 = assetLoader('logo2');
    logo2.x = getWidthCenter(logo2);
    logo2.y = 80;
    logo2.alpha = 0;
    stage.addChild(logo2);

    var people1_2 = assetLoader('people1_2');
    people1_2.x = getWidthCenter(people1_2);
    people1_2.y = 230;
    people1_2.alpha = 0;
    stage.addChild(people1_2);

    var action = assetLoader('action');
    action.x = 580;
    action.y = 200;
    action.alpha = 0;
    stage.addChildAt(action, stage.getChildIndex(COMMON.computer));

    //var shadow2 = new createjs.Shape();
    //shadow2.graphics
    //.beginLinearGradientFill(["#4158db","#2494e2"], [0, 1], 100, 50, 230, 0)
    //.moveTo(0,0)
    //.bezierCurveTo(135,-44,205,-27,230,0)
    //.bezierCurveTo(210,-10,144,74,145,103)
    //.lineTo(0,0);
    //shadow2.alpha = 0.45;
    //shadow2.x = 82;
    //shadow2.y = 140;
    //stage.addChild(shadow2);

    var mask3 = new SpriteMask(280,-190,250,200);
    mask3.rotation = 40;
    // mask3.alpha = 0.8;
    stage.addChild(mask3);

    //shadow2.mask = mask3;

    var textBox2_Show = function(){
        var tl = new TimelineMax();
        tl.to('#text2', 0, {alpha:0, scale:0.1,x:"+=150",y:"-=60"},0)
        .to('#text2',0.8,{alpha:0.9,scale:1,x:"-=150",y:"+=60"},0)
        .to(mask3, 0.8,{x:"-=100",y:"+=152"},0);
        return tl;
    }

    var textBox2_Exit = function(){
        var tl = new TimelineMax();
        tl.to(mask3, 0.8,{x:"+=100",y:"-=152"},0)
        .to('#text2', 0.8, {alpha:0, scale:0.1,x:"+=150",y:"-=60"},0);
        return tl;
    }

    var tl = new TimelineMax();
    tl.to(background2, 0.8, {alpha:1,delay:2})
    .to(logo2, 0.8, {alpha:1,repeat:2,yoyo:true})
    .add(textBox2_Show());

    tl.add(textBox2_Exit(),"+="+3)
    .to(logo2, 0.8, {alpha:0})
    .to(background2, 0.8, {alpha:0});

    return tl;
}

function Stage3(){
    stage_progress = 3;
    console.debug("=================stage_progress" + stage_progress);
    var background3 = new assetLoader("background3");
    background3.x = 575;
    background3.y = 34 - background3.image.height;
    stage.addChild(background3);

    background3.mask = COMMON.maskScreen;

    var logo3 = new assetLoader("logo3");
    logo3.x = 450;
    logo3.y = -100;
    stage.addChild(logo3);

    logo3.mask = COMMON.maskScreen;

    var people2_3 = new assetLoader("people2_3");
    people2_3.x = 750;
    people2_3.y = 80;
    stage.addChild(people2_3);

    var textBox3_Show = function(){
        var tl = new TimelineMax();
        tl.to('#text3', 0.8, {alpha:0.9});
        return tl;
    }

    var textBox3_Exit = function(){
        var tl = new TimelineMax();
        tl.to('#text3', 0.8, {alpha:0});
        return tl;
    }

    var tl = new TimelineMax();

    background3.alpha = 0;
    background3.x = COMMON.screen.x;
    background3.y = COMMON.screen.y;
    logo3.alpha = 0;
    logo3.y+= 130;

    tl.to(background3, 1.2, {alpha:1,delay:2})
    .to(logo3, 0.8, {alpha:1})
    .add(textBox3_Show());

    tl.add(textBox3_Exit(),"+="+3)
    .to(logo3, 0.8, {alpha:0})
    .to(background3, 0.8, {alpha:0});
    return tl;
}

function Stage4(){
    stage_progress = 4;
    console.debug("=================stage_progress" + stage_progress);
    var background5_1 = new assetLoader('background5_1');
    background5_1.x = 174;
    background5_1.y = 34 - background5_1.image.height;
    stage.addChild(background5_1);

    background5_1.mask = COMMON.maskScreen;

    var background5_2 = new assetLoader('background5_2');
    background5_2.x = getWidthCenter(background5_2);
    background5_2.y = getHeightCenter(background5_2);
    background5_2.alpha = 0;
    stage.addChildAt(background5_2, null);

    var people4_5 = new assetLoader('people4_5');
    people4_5.x = 370;
    people4_5.y = 445;
    stage.addChild(people4_5);

    var people5_5 = new assetLoader('people5_5');
    people5_5.x = 532;
    people5_5.y = 150;
    people5_5.alpha = 0;
    stage.addChild(people5_5);

    var people6_5 = new assetLoader('people6_5');
    people6_5.x = -140;
    people6_5.y = 77;
    stage.addChild(people6_5);

    var logo4 = new assetLoader('logo4');
    logo4.x = getWidthCenter(logo4);
    logo4.y = getHeightCenter(logo4) - 80;
    stage.addChild(logo4);

    var mask5 = new SpriteMask(312,-35,128,128);
    stage.addChild(mask5);

    logo4.mask = mask5;

    //var shadow4 = new createjs.Shape();
    //shadow4.graphics
    //.beginLinearGradientFill(["#58d0ff","#2494e2"], [0, 1], 376, 151, 21, 411)
    //.moveTo(352,0)
    //.lineTo(0,154)
    //.lineTo(304,272)
    //.lineTo(352,0);
    //shadow4.alpha = 0.45;
    //shadow4.x = 21;
    //shadow4.y = 144;
    //stage.addChild(shadow4);

    var mask6 = new SpriteMask(230,-158,460,300);
    stage.addChild(mask6);

    //shadow4.mask = mask6;

    var textBox5_Show = function(){
        var tl = new TimelineMax();
        tl.to('#text4', 0, {alpha:0, scale:0.1,x:"+=200",y:"-=205"})
        .to('#text4', 0.8, {alpha:0.95,scale:1,x:"-=200",y:"+=205"},0)
        .to(mask6, 1.2, {x:"-=260",y:"+=300"},0)
        return tl;
    }

    var tl = new TimelineMax();

    background5_1.alpha = 0;
    background5_1.y = COMMON.screen.y;

    tl.to(background5_1, 0.8, {alpha:1,delay:2})
    .to(background5_2, 0.8, {alpha:1})
    .to(mask5, 0.8, {y:"+=128"})
    .add(textBox5_Show());

    return tl;
}

function restartStage(){
	var tl = new TimelineMax();
	tl.to(COMMON.computer, 0, {delay:5,onComplete:function(){
		mainline.restart();
        stage_progress = 0;
	}});
	return tl;
}

function handleComplete(){
    console.log("handleComplete=============");
    stage_progress = 0;
    if (fileCount < manifest.length){
        console.log("handleFileLoad lost file=============");
        window.location.reload();
        return;
    }
    console.log("handleFileLoad finish=============");
    setTimeout(function() {
        createjs.Ticker.addEventListener("tick", handleTick);
        function handleTick() {
            stage.update();
        }

        var mainline = new TimelineMax();
        mainline
        .add(Stage1())
        .add(Stage2())
        .add(Stage3())
        .add(Stage4())
        .add(restartStage());

        window.mainline = mainline;
    },500);
}

function handleReload(){
    console.log("handleReload=============");
    window.location.reload();
}
function handleFileLoad(){
    fileCount++;
    console.log("handleFileLoad=============" + fileCount );
}

document.body.onload = function() {
    console.debug("===========iframe document.body.onload");
    setTimeout(function() {
        canvas = document.getElementById('slideStage'),
        stage  = new createjs.Stage(canvas),
        loader = new createjs.LoadQueue(false),
        loader.addEventListener('fileload', handleFileLoad);
        loader.addEventListener('complete', handleComplete);
        loader.addEventListener('error', handleReload);
        loader.addEventListener('interrupted', handleReload);
        loader.addEventListener('failed', handleReload);
        loader.loadManifest(manifest);
    },1000);
};

document.body.onerror = function() {
    console.debug("===========iframe document.body.onerror");
    window.location.reload()
};
