import { initializeApp } from "https://www.gstatic.com/firebasejs/10.5.0/firebase-app.js";
import { getDatabase, ref, push, get, child } from "https://www.gstatic.com/firebasejs/10.5.0/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyChMG3yyK6WXnLhc-QgoL9CYcA62APqyIg",
  authDomain: "portaeletronica-a6cfe.firebaseapp.com",
  databaseURL: "https://portaeletronica-a6cfe-default-rtdb.firebaseio.com",
  projectId: "portaeletronica-a6cfe",
  storageBucket: "portaeletronica-a6cfe.firebasestorage.app",
  messagingSenderId: "162180097100",
  appId: "1:162180097100:web:e2673dafba5e7d0aee00e9",
  measurementId: "G-0HZFXHWBHR",
};

const app = initializeApp(firebaseConfig);
const database = getDatabase(app);
const dbRef = ref(database);

const botao = document.getElementById("btnCadastrar");
const icone = document.getElementById("btnIcone");
const texto = document.getElementById("btnTexto");

document.getElementById("loginForm").addEventListener("submit", async function (event) {
  event.preventDefault();

  const username = document.getElementById("username").value;
  const pin = document.getElementById("pin").value;

  const regexUsername = /^[CRUD\d]{4}$/;
  const regexPin = /^\d{4}$/;

  if (username && regexUsername.test(username) && regexPin.test(pin)) {
    try {
      const snapshot = await get(child(dbRef, "user"));
      let existe = false;

      if (snapshot.exists()) {
        const dados = snapshot.val();
        for (let key in dados) {
          if (dados[key].username === username) {
            existe = true;
            break;
          }
        }
      }

      if (existe) {
        botao.classList.remove("sucesso");
        botao.classList.add("erro");
        icone.className = "fas fa-times";
        texto.textContent = "Usuário já existe";
        document.getElementById("mensagem").textContent = `❌ O usuário "${username}" já está cadastrado.`;
      } else {
        push(ref(database, "user"), {
          username: username,
          pin: pin,
          timestamp: new Date().toISOString(),
        });

        botao.classList.remove("erro");
        botao.classList.add("sucesso");
        icone.className = "fas fa-check";
        texto.textContent = "Cadastrado";
        document.getElementById("mensagem").textContent = `✅ Usuário "${username}" cadastrado com sucesso!`;
      }

      setTimeout(() => {
        botao.classList.remove("sucesso", "erro");
        icone.className = "fas fa-circle-notch";
        texto.textContent = "Cadastrar";
      }, 3000);
    } catch (error) {
      console.error("Erro ao verificar usuário:", error);
    }
  } else {
    botao.classList.remove("sucesso");
    botao.classList.add("erro");
    icone.className = "fas fa-times";
    texto.textContent = "Erro";
    document.getElementById("mensagem").textContent = `❌ Dados inválidos. Verifique o usuário e o PIN.`;

    setTimeout(() => {
      botao.classList.remove("sucesso", "erro");
      icone.className = "fas fa-circle-notch";
      texto.textContent = "Cadastrar";
    }, 3000);
  }
});