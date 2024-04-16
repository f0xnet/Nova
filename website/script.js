// script.js
document.addEventListener('DOMContentLoaded', function() {
    const posts = document.getElementById('posts');

    function addPost(title, content) {
        const postElement = document.createElement('div');
        postElement.innerHTML = `<h3>${title}</h3><p>${content}</p>`;
        posts.appendChild(postElement);
    }

    // Exemple d'ajout de posts
    addPost('Post Introductif', 'Bienvenue sur notre blog rétro. Ici, nous partageons des anecdotes et des histoires inspirées des années 80.');
    addPost('Cyberpunk 1980', 'Exploration du mouvement Cyberpunk dans les années 80 et son impact sur la culture et la technologie.');

    // Ajouter une interface pour saisir de nouveaux posts ici si nécessaire
});
