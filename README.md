# Game Project Workflow - CMSC 123

## Steps to Start Working on the Game Project

### 1. Clone the Repository
- Open your IDE (e.g., VS Code, IntelliJ, or your preferred tool).
- In the terminal, navigate to the folder where you want to clone the repository.
- Run the following command to clone the project:

    ```bash
    git clone <repository-url>
    ```

    Replace `<repository-url>` with the actual URL of the repository (e.g., from GitHub or GitLab). This will create a local copy of the project on your machine.

### 2. Create a New Branch
- In the issues section of the repository, there's a button for creating a branch.
- Click on it, and it will generate the necessary Git command for you.
- Copy the command and paste it into your IDE's terminal to create your new branch.
  
    This ensures you work on a separate branch without affecting the main codebase.

### 3. Resolve Issues
- Once you're on your new branch, start addressing the issue or feature assigned to you.
- Make sure to write clean, understandable code, and commit your changes frequently with clear commit messages. Example commit message:

    ```bash
    git commit -m "Fixed bug in player movement logic"
    ```

### 4. Push Changes
- Once you've resolved the issue or completed the feature, push your changes to the remote repository:

    ```bash
    git push origin <branch-name>
    ```

    This will upload your branch to the remote repository so that others can review your work.

### 5. Create a Pull Request (PR)
- Go to the repository on the web (GitHub, GitLab, etc.), and you should see an option to create a pull request for your branch.
- Create a PR for your branch, providing a clear description of the changes you've made.
- Assign reviewers from your team and request feedback.

### 6. Merge Once Approved
- Once your PR is reviewed and approved by the team, it can be merged into the main branch.
- Make sure to resolve any merge conflicts if they arise, and pull the latest changes to keep your local version updated.

---

Follow this process to ensure smooth collaboration and version control throughout the development of the project.
