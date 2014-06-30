
//personal
var ParticleScale = 0.7//default = 0.7
var ParticleSpeed = 200//defalut = 100
var ZStart = 3000;//default = 3000
var ZEnd = 70;//default = 70
var UpdateTimes = 15;//每秒渲染次，及星星个数  default = 15
var ZCamera = 1000;//相机的位置default = 1000


//定义应用所需的组件:相机,场景,渲染器
var camera, scene, renderer;
var _b = document.body;
//跟踪鼠标的位置
var mouseX = _b.offsetWidth / 2, mouseY = _b.offsetHeight / 2;
//定义存储粒子的数组
var particles = [];

//数据初始化
function init(){
    console.log("sky theme init==== document.body.onload");
    //document.body.style.backgroundImage = "url(js/skyThree/sky.jpg)"; 
    //相机参数：
    //四个参数值分别代表:视野角：fov  纵横比：aspect 相机离视体最近的距离：near 相机离视体最远的距离：far
    camera = new THREE.PerspectiveCamera(80, window.innerWidth / window.innerHeight, ZEnd, ZStart );
    //设置相机位置,默认位置为:0,0,0.
    camera.position.z = ZCamera;

    //声明场景
    scene = new THREE.Scene();
    //将相机装加载到场景
    scene.add(camera);

    //生成渲染器的对象
    renderer = new THREE.CanvasRenderer();
    //设置渲染器的大小
    renderer.setSize( window.innerWidth, window.innerHeight );
    //追加元素
    document.body.appendChild(renderer.domElement);
    //调用自定义的生成粒子的方法
    makeParticles();
    //添加鼠标移动监听
    document.body.addEventListener('mousemove',onMouseMove,false);
    //设置间隔调用update函数,间隔次数为每秒30次
    setInterval(update,1000/UpdateTimes);
}
 
function update() {
    mouseY += 50;
    if(mouseY > 768) mouseY = 768;
    //调用移动粒子的函数
    updateParticles();
    //重新渲染
    renderer.render( scene, camera );
}

//定义粒子生成的方法
function makeParticles(){
    console.log("makeParticles");
    var particle,material;
    //粒子从Z轴产生区间在-1000到1000
    for(var zpos=-1000;zpos<1000;zpos+=20){
        //we make a particle material and pass through the colour and custom particle render function we defined.
        material = new THREE.ParticleCanvasMaterial( { color: 0xffffff, program: particleRender } );
        material.opacity= 0.85;
        material.transparent = true;

        //生成粒子
        particle = new THREE.Particle(material);
        //随即产生x轴,y轴,区间值为-500-500
        particle.position.x = Math.random()*1000-500; //math . random()返回一个浮点数在0和1之间
        particle.position.y = Math.random()*1000-500;
        //设置z轴
        particle.position.z = zpos;
        //scale it up a bit
        particle.scale.x = particle.scale.y = ParticleScale;
        //将产生的粒子添加到场景
        scene.add(particle);
        //将粒子位置的值保存到数组
        particles.push(particle);
    }
}

//定义粒子渲染器
function particleRender( context ) {
    //获取canvas上下文的引用
    context.beginPath();
    // and we just have to draw our shape at 0,0 - in this
    // case an arc from 0 to 2Pi radians or 360º - a full circle!
    context.arc( 0, 0, 1, 0,  Math.PI * 2, true );
    //设置原型填充
    context.fill();
}

     
//移动粒子的函数
function updateParticles(){
    //遍历每个粒子
    for(var i=0; i<particles.length; i++){
        particle = particles[i];
        //设置粒子向前移动的速度依赖于鼠标在平面Y轴上的距离
        particle.position.z +=  mouseY * ParticleSpeed / 10000;
        //如果粒子Z轴位置到1000,将z轴位置设置到-1000
        if(particle.position.z>1000){
             particle.position.z-=2000;
        }
    }
}
 
//鼠标移动时调用
function onMouseMove(event){
    mouseX = event.clientX;
    mouseY = event.clientY;
}

document.body.onload = init();
//init();
